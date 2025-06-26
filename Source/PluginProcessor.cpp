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

    // Parámetro para la velocidad (en Hz)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "lfoSpeed", "LFO Speed",
        juce::NormalisableRange<float>(0.1f, 5.0f, 0.01f),  // Rango: 0.1Hz a 5Hz
        0.5f));  // Valor por defecto: 0.5Hz (ciclo cada 2 segundos)


    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "minFreq", "Minimum Frequency",
        juce::NormalisableRange<float>(50.0f, 2000.0f, 1.0f), 500.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "maxFreq", "Maximum Frequency",
        juce::NormalisableRange<float>(50.0f, 2000.0f, 1.0f), 500.0f));

    return { params.begin(), params.end() };
}

// Implementaciones faltantes:
void SirenAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
    lfoPhase = 0.0f;
    squarePhase = 0.0f;
}

void SirenAudioProcessor::releaseResources()
{
    // Liberar recursos si es necesario
}

void SirenAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    const float minFreq = apvts->getRawParameterValue("minFreq")->load();
    const float maxFreq = apvts->getRawParameterValue("maxFreq")->load();
    const float lfoSpeed = apvts->getRawParameterValue("lfoSpeed")->load();

    const int numSamples = buffer.getNumSamples();

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int i = 0; i < numSamples; ++i)
        {
            // Velocidad del cambio de pitch
            lfoPhase += lfoSpeed / currentSampleRate;  
            if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
            //juce::Logger::writeToLog("LFO Speed: " + juce::String(lfoSpeed));


            // 1. Actualizar LFO (sinusoidal)
            lfoPhase += lfoFreq / currentSampleRate;
            if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

            // 2. Calcular frecuencia actual
            const float lfoValue = std::sin(2.0f * juce::MathConstants<float>::pi * lfoPhase);
            const float currentFreq = minFreq + (maxFreq - minFreq) * (0.5f + 0.5f * lfoValue);

            // 3. Generar onda cuadrada
            squarePhase += currentFreq / currentSampleRate;
            if (squarePhase >= 1.0f) squarePhase -= 1.0f;

            //Onda cuadrada
            //channelData[i] = (squarePhase < 0.5f) ? 0.8f : -0.8f;
            //Onda sinusoidal
            channelData[i] = 0.8f * std::sin(2.0f * juce::MathConstants<float>::pi * squarePhase); 

        }
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