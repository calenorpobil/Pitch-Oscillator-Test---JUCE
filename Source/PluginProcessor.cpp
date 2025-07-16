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
    
    // Nuevo parámetro para el volumen (75% por defecto)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gain", "Volume", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.75f));

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
        false
    ));
    oversampler->initProcessing(static_cast<size_t>(samplesPerBlock));

    // Configurar osciladores
    auto oversampledRate = sampleRate * oversampler->getOversamplingFactor();
    auto maxBlockSize = static_cast<juce::uint32>(samplesPerBlock * oversampler->getOversamplingFactor());

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
        static_cast<juce::uint32>(getTotalNumOutputChannels())  // Conversión segura
        });
    mainOscillator.setFrequency(500.0f);

    // Configurar suavizado
    const double rampTimeSec = 0.05;
    smoothedMinFreq.reset(currentSampleRate, rampTimeSec);
    smoothedMaxFreq.reset(currentSampleRate, rampTimeSec);
    smoothedLfoSpeed.reset(currentSampleRate, rampTimeSec);
    smoothedGain.reset(currentSampleRate, rampTimeSec);
}

void SirenAudioProcessor::releaseResources()
{
    oversampler->reset();
    lfoOscillator.reset();
    mainOscillator.reset();
}

void SirenAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // Actualizar valores suavizados
    smoothedMinFreq.setTargetValue(apvts->getRawParameterValue("minFreq")->load());
    smoothedMaxFreq.setTargetValue(apvts->getRawParameterValue("maxFreq")->load());
    smoothedLfoSpeed.setTargetValue(apvts->getRawParameterValue("lfoSpeed")->load());
    smoothedGain.setTargetValue(apvts->getRawParameterValue("gain")->load());

    // Procesamiento con oversampling
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> oversampledBlock;

    if (oversamplingEnabled) {
        oversampledBlock = oversampler->processSamplesUp(block);
    }
    else {
        oversampledBlock = block;
    }

    const auto osSampleRate = currentSampleRate * oversampler->getOversamplingFactor();
    const size_t osNumSamples = oversampledBlock.getNumSamples();

    // Buffer para procesamiento mono del LFO
    juce::AudioBuffer<float> lfoBuffer(1, osNumSamples);

    // Procesar LFO (mono)
    for (int sample = 0; sample < osNumSamples; ++sample) {
        lfoBuffer.setSample(0, sample, lfoOscillator.processSample(0.0f));
        lfoOscillator.setFrequency(smoothedLfoSpeed.getNextValue());
    }

    // Procesar oscilador principal (por canal)
    for (int channel = 0; channel < oversampledBlock.getNumChannels(); ++channel) {
        auto* channelData = oversampledBlock.getChannelPointer(channel);

        for (int sample = 0; sample < osNumSamples; ++sample) {
            // Calcular frecuencia actual
            const float minFreq = smoothedMinFreq.getNextValue();
            const float maxFreq = smoothedMaxFreq.getNextValue();
            const float lfoValue = lfoBuffer.getSample(0, sample);
            const float currentFreq = minFreq + (maxFreq - minFreq) * (0.5f + 0.5f * lfoValue);

            // Actualizar frecuencia del oscilador principal
            mainOscillator.setFrequency(currentFreq);

            // Procesar muestra
            const float gain = smoothedGain.getNextValue();
            channelData[sample] = gain * mainOscillator.processSample(0.0f);
        }
    }

    // Downsampling
    if (oversamplingEnabled) {
        oversampler->processSamplesDown(block);
    }
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