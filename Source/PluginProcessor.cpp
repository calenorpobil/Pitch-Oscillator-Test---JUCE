/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TapSynthAudioProcessor::TapSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    synth.addSound(new SynthSound());
    synth.addVoice(new SynthVoice());

    apvts.reset(new juce::AudioProcessorValueTreeState(*this, nullptr, "Parameters", createParameters()));
}

TapSynthAudioProcessor::~TapSynthAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout TapSynthAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

<<<<<<< HEAD
    // Partro para la velocidad (en Hz)
=======
>>>>>>> 1df56f85feed69e1db2e64bbe0578cc1b23ab7aa
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

<<<<<<< HEAD
//==============================================================================
const juce::String TapSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
=======
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
>>>>>>> 1df56f85feed69e1db2e64bbe0578cc1b23ab7aa
}

bool TapSynthAudioProcessor::acceptsMidi() const
{
<<<<<<< HEAD
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TapSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TapSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TapSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TapSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TapSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TapSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TapSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void TapSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TapSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{


    synth.setCurrentPlaybackSampleRate(sampleRate);

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
            voice->prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    }

    // Toca la nota C4 al arrancar
    const int midiChannel = 1;
    const int midiNoteNumber = 20; // C4
    const float velocity = 0.2f;

    synth.noteOn(midiChannel, midiNoteNumber, velocity);

}


void TapSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TapSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TapSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
=======
    oversampler->reset();
    lfoOscillator.reset();
    mainOscillator.reset();
    antiAliasingFilter.reset();
}

void SirenAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
>>>>>>> 1df56f85feed69e1db2e64bbe0578cc1b23ab7aa
{
    // 1. Limpiar buffers
    buffer.clear();

<<<<<<< HEAD
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    const int numSamples = buffer.getNumSamples();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear (i, 0, numSamples);
        
        for (int i = 0; i < numSamples; ++i)
        {
            // Velocidad del cambio de pitch
            lfoPhase += lfoSpeed / currentSampleRate;  
            if (lfoPhase >= 1.0f) 
                lfoPhase -= 1.0f;
            //juce::Logger::writeToLog("LFO Speed: " + juce::String(lfoSpeed));
=======
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
>>>>>>> 1df56f85feed69e1db2e64bbe0578cc1b23ab7aa

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

<<<<<<< HEAD
            //Onda cuadrada
            //channelData[i] = (squarePhase < 0.5f) ? 0.8f : -0.8f;
            //Onda sinusoidal
=======
            const float waveValue = std::sin(juce::MathConstants<float>::twoPi * mainPhase);
>>>>>>> 1df56f85feed69e1db2e64bbe0578cc1b23ab7aa

            // Aplicar ganancia con protección
            const float gain = smoothedGain.getNextValue();
            float sampleValue = waveValue * gain;

            // Soft clipping y limitación
            sampleValue = std::tanh(sampleValue * 0.8f);
            sampleValue = juce::jlimit(-0.95f, 0.95f, sampleValue);

            channelData[sample] = sampleValue;
        }
    }

<<<<<<< HEAD
    
        

    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
=======
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
>>>>>>> 1df56f85feed69e1db2e64bbe0578cc1b23ab7aa
}


//==============================================================================
bool TapSynthAudioProcessor::hasEditor() const
{
<<<<<<< HEAD
    return true; // (change this to false if you choose to not supply an editor)
=======
    return new SirenAudioProcessorEditor(*this); // Asegúrate de que el editor esté correctamente implementado
>>>>>>> 1df56f85feed69e1db2e64bbe0578cc1b23ab7aa
}

juce::AudioProcessorEditor* TapSynthAudioProcessor::createEditor()
{
    return new TapSynthAudioProcessorEditor (*this);
}

//==============================================================================
void TapSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TapSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapSynthAudioProcessor();
}
