/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SynthSound.h"
#include "SynthVoice.h"

//==============================================================================
/**
*/
class TapSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    TapSynthAudioProcessor();
    ~TapSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

<<<<<<< HEAD
   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
=======
    float calculateSmoothedFrequency(int sample, int totalSamples);

    bool hasEditor() const override { return true; }
>>>>>>> 1df56f85feed69e1db2e64bbe0578cc1b23ab7aa
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getState() { return *apvts; }

private:

    juce::Synthesiser synth;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState> apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

<<<<<<< HEAD
    bool shouldPlayNote = true;
    const int midiChannel = 1;
    const int midiNoteNumber = 60; // C4
    const float velocity = 0.8f;
    
=======
    // Osciladores DSP
    juce::dsp::Oscillator<float> lfoOscillator;  // Oscilador LFO
    juce::dsp::Oscillator<float> mainOscillator; // Oscilador principal

    // Sistema de oversampling
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

>>>>>>> 1df56f85feed69e1db2e64bbe0578cc1b23ab7aa
    float lfoPhase = 0.0f;
    float oscillatorPhase = 0.0f;
    double currentSampleRate = 44100.0;
<<<<<<< HEAD
    const float lfoFreq = 0.5f;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapSynthAudioProcessor)
};
=======
    float mainPhase = 0.0f;
    float currentMainFreq = 500.0f;
    double lastSampleRate = 0.0;
    
    // Parámetros suavizados
    juce::SmoothedValue<float> smoothedMinFreq{ 500.0f };
    juce::SmoothedValue<float> smoothedMaxFreq{ 500.0f };
    juce::SmoothedValue<float> smoothedLfoSpeed{ 0.5f };
    juce::SmoothedValue<float> smoothedGain{ 0.75f };  // Volumen al 75% por defecto

    // Filtro anti-aliasing
    juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>> antiAliasingFilter;


    bool oversamplingEnabled = true;  // Habilitar/deshabilitar oversampling

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SirenAudioProcessor)
};
>>>>>>> 1df56f85feed69e1db2e64bbe0578cc1b23ab7aa
