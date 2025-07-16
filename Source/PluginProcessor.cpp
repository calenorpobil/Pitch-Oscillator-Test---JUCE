#include "PluginProcessor.h"
#include "PluginEditor.h"

SirenAudioProcessor::SirenAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    apvts.reset(new juce::AudioProcessorValueTreeState(*this, nullptr, "Parameters", createParameters()));
}

juce::AudioProcessorValueTreeState::ParameterLayout SirenAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "lfoSpeed", "LFO Speed", juce::NormalisableRange<float>(0.1f, 5.0f, 0.01f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "minFreq", "Minimum Frequency", juce::NormalisableRange<float>(50.0f, 2000.0f, 1.0f), 500.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "maxFreq", "Maximum Frequency", juce::NormalisableRange<float>(50.0f, 2000.0f, 1.0f), 500.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gain", "Volume", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.75f));

    // Nuevo parámetro para el oversampling
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "oversampling", "Oversampling", true));

    return { params.begin(), params.end() };
}

void SirenAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Configurar oversampling (4x)
    oversampler.reset(new juce::dsp::Oversampling<float>(
        getTotalNumOutputChannels(),
        2,  // 2 etapas = 4x oversampling
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true // Habilita filtrado adicional
    ));
    oversampler->initProcessing(static_cast<size_t>(samplesPerBlock));

    // Configurar osciladores
    auto oversampledRate = sampleRate * oversampler->getOversamplingFactor();
    auto maxBlockSize = static_cast<juce::uint32>(samplesPerBlock * oversampler->getOversamplingFactor());

    // Configurar filtro anti-aliasing
    *antiAliasingFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
        oversampledRate,
        currentSampleRate * 0.45f  // Frecuencia de corte al 45% de Nyquist
    );
    antiAliasingFilter.prepare({ oversampledRate, maxBlockSize, static_cast<juce::uint32>(getTotalNumOutputChannels()) });

    // Configurar LFO (sinusoidal)
    lfoOscillator.initialise([](float x) { return std::sin(x); });
    lfoOscillator.prepare({
        oversampledRate,
        maxBlockSize,
        1  // 1 canal (mono)
        });
    lfoOscillator.setFrequency(0.5f);

    // Configurar oscilador principal (sinusoidal)
    mainOscillator.initialise([](float x) { return std::sin(x); });
    mainOscillator.prepare({
        oversampledRate,
        maxBlockSize,
        static_cast<juce::uint32>(getTotalNumOutputChannels())
        });
    mainOscillator.setFrequency(500.0f);

    // Configurar suavizado
    const double rampTimeSec = 0.05;
    smoothedMinFreq.reset(currentSampleRate, rampTimeSec);
    smoothedMaxFreq.reset(currentSampleRate, rampTimeSec);
    smoothedLfoSpeed.reset(currentSampleRate, rampTimeSec);
    smoothedGain.reset(currentSampleRate, rampTimeSec);

    // Obtener estado inicial del oversampling
    oversamplingEnabled = apvts->getRawParameterValue("oversampling")->load();
}

void SirenAudioProcessor::releaseResources()
{
    oversampler->reset();
    lfoOscillator.reset();
    mainOscillator.reset();
    antiAliasingFilter.reset();
}

void SirenAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // 1. Actualizar estado del oversampling
    oversamplingEnabled = apvts->getRawParameterValue("oversampling")->load();
    const float rawGain = apvts->getRawParameterValue("gain")->load();

    //Gestión de ganancia
    const float targetGain = juce::Decibels::decibelsToGain(rawGain * 100.0f - 30.0f); // Rango útil: -30dB a +20dB
    smoothedGain.setTargetValue(targetGain);

    // 2. Actualizar valores suavizados
    smoothedMinFreq.setTargetValue(apvts->getRawParameterValue("minFreq")->load());
    smoothedMaxFreq.setTargetValue(apvts->getRawParameterValue("maxFreq")->load());
    smoothedLfoSpeed.setTargetValue(apvts->getRawParameterValue("lfoSpeed")->load());
    smoothedGain.setTargetValue(apvts->getRawParameterValue("gain")->load());

    // 3. Procesamiento con oversampling
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> osBlock = block;

    if (oversamplingEnabled) {
        osBlock = oversampler->processSamplesUp(block);
    }

    const auto osSampleRate = currentSampleRate * (oversamplingEnabled ? oversampler->getOversamplingFactor() : 1);
    const int osNumSamples = osBlock.getNumSamples();

    // 4. Procesar LFO (mono)
    juce::AudioBuffer<float> lfoBuffer(1, osNumSamples);
    for (int sample = 0; sample < osNumSamples; ++sample) {
        lfoBuffer.setSample(0, sample, lfoOscillator.processSample(0.0f));
        lfoOscillator.setFrequency(smoothedLfoSpeed.getNextValue());
    }

    // 5. Procesar oscilador principal
    for (int channel = 0; channel < osBlock.getNumChannels(); ++channel)
    {
        auto* channelData = osBlock.getChannelPointer(channel);

        for (int sample = 0; sample < osNumSamples; ++sample)
        {
            // Cálculo de frecuencia con suavizado adicional
            const float currentFreq = calculateSmoothedFrequency(sample, osNumSamples);
            mainOscillator.setFrequency(currentFreq);

            // Procesamiento con wave shaping suave
            float sampleValue = mainOscillator.processSample(0.0f);
            sampleValue = std::tanh(sampleValue * 0.8f); // Soft clipping

            // Aplicación de ganancia con protección contra clipping
            const float gain = smoothedGain.getNextValue();
            channelData[sample] = juce::jlimit(-0.95f, 0.95f, sampleValue * gain);
        }
    }

    if (!oversamplingEnabled) {
        *antiAliasingFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
            currentSampleRate,
            currentSampleRate * 0.45f
        );
        antiAliasingFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
    }

    // 6. Aplicar filtro anti-aliasing si está habilitado el oversampling
    if (oversamplingEnabled) {
        antiAliasingFilter.process(juce::dsp::ProcessContextReplacing<float>(osBlock));
        oversampler->processSamplesDown(block);
    }
}

float SirenAudioProcessor::calculateSmoothedFrequency(int sample, int totalSamples)
{
    const float minFreq = smoothedMinFreq.getNextValue();
    const float maxFreq = smoothedMaxFreq.getNextValue();
    const float lfoValue = lfoOscillator.processSample(0.0f);

    // Suavizado adicional para evitar saltos bruscos
    const float normalizedPos = sample / static_cast<float>(totalSamples);
    const float frequency = minFreq + (maxFreq - minFreq) * (0.5f + 0.5f * lfoValue);

    return frequency * (1.0f - normalizedPos) + frequency * normalizedPos;
}

juce::AudioProcessorEditor* SirenAudioProcessor::createEditor()
{
    return new SirenAudioProcessorEditor(*this); // Asegúrate de que el editor esté correctamente implementado
}

void SirenAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts->copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SirenAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        apvts->replaceState(juce::ValueTree::fromXml(*xmlState));
}