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
    // Reiniciar todo al cambiar sample rate
    if (std::abs(sampleRate - lastSampleRate) > 1.0)
    {
        lfoPhase = 0.0f;
        mainPhase = 0.0f;
        lastSampleRate = sampleRate;
    }

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
    const auto osSampleRate = sampleRate * oversampler->getOversamplingFactor();
    // Configurar filtros
    *antiAliasingFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
        osSampleRate,
        sampleRate * 0.45f
    );
    antiAliasingFilter.prepare({ osSampleRate, (juce::uint32)samplesPerBlock,
                               static_cast<juce::uint32>(getTotalNumOutputChannels()) });


    // Configurar suavizado con tiempos fijos (no dependientes de sample rate)
    smoothedMinFreq.reset(50.0);  // Tiempo en ms
    smoothedMaxFreq.reset(50.0);
    smoothedLfoSpeed.reset(50.0);
    smoothedGain.reset(50.0);
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
    // 1. Limpiar buffers
    buffer.clear();

    // 2. Actualizar parámetros
    oversamplingEnabled = apvts->getRawParameterValue("oversampling")->load();
    const float rawGain = apvts->getRawParameterValue("gain")->load();
    const float targetGain = juce::Decibels::decibelsToGain(rawGain * 50.0f - 30.0f); // -30dB a +20dB
    smoothedGain.setTargetValue(targetGain);

    smoothedMinFreq.setTargetValue(apvts->getRawParameterValue("minFreq")->load());
    smoothedMaxFreq.setTargetValue(apvts->getRawParameterValue("maxFreq")->load());
    smoothedLfoSpeed.setTargetValue(apvts->getRawParameterValue("lfoSpeed")->load());

    // 3. Manejo de oversampling
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> osBlock = block;

    if (oversamplingEnabled)
    {
        osBlock = oversampler->processSamplesUp(block);
    }

    const int osNumSamples = osBlock.getNumSamples();
    const double osSampleRate = currentSampleRate * (oversamplingEnabled ? oversampler->getOversamplingFactor() : 1);

    // 4. Procesamiento de audio con gestión manual de fase
    for (int channel = 0; channel < osBlock.getNumChannels(); ++channel)
    {
        auto* channelData = osBlock.getChannelPointer(channel);

        for (int sample = 0; sample < osNumSamples; ++sample)
        {
            // Actualizar LFO (fase continua)
            lfoPhase += smoothedLfoSpeed.getNextValue() / osSampleRate;
            if (lfoPhase > 1.0f) lfoPhase -= 1.0f;

            const float lfoValue = std::sin(juce::MathConstants<float>::twoPi * lfoPhase);

            // Calcular frecuencia con suavizado
            const float minFreq = smoothedMinFreq.getNextValue();
            const float maxFreq = smoothedMaxFreq.getNextValue();
            const float targetFreq = minFreq + (maxFreq - minFreq) * (0.5f + 0.5f * lfoValue);

            // Suavizado de frecuencia adicional
            currentMainFreq = 0.99f * currentMainFreq + 0.01f * targetFreq;

            // Actualizar oscilador principal
            mainPhase += currentMainFreq / osSampleRate;
            if (mainPhase > 1.0f) mainPhase -= 1.0f;

            const float waveValue = std::sin(juce::MathConstants<float>::twoPi * mainPhase);

            // Aplicar ganancia con protección
            const float gain = smoothedGain.getNextValue();
            float sampleValue = waveValue * gain;

            // Soft clipping y limitación
            sampleValue = std::tanh(sampleValue * 0.8f);
            sampleValue = juce::jlimit(-0.95f, 0.95f, sampleValue);

            channelData[sample] = sampleValue;
        }
    }

    // 5. Procesamiento post-oversampling
    if (oversamplingEnabled)
    {
        antiAliasingFilter.process(juce::dsp::ProcessContextReplacing<float>(osBlock));
        oversampler->processSamplesDown(block);
    }


    // 7. Normalización final de ganancia
    const float outputGain = juce::Decibels::decibelsToGain(-3.0f); // -3dB de headroom
    buffer.applyGain(outputGain);
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