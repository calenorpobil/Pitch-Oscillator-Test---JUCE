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

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
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

    bool shouldPlayNote = true;
    const int midiChannel = 1;
    const int midiNoteNumber = 60; // C4
    const float velocity = 0.8f;
    
    float lfoPhase = 0.0f;
    float squarePhase = 0.0f;
    double currentSampleRate = 44100.0;
    const float lfoFreq = 0.5f;

    float minFreq;
    float maxFreq;
    float lfoSpeed;

    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapSynthAudioProcessor)
};
