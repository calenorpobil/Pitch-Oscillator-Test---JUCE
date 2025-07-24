/*
  ==============================================================================

    SynthVoice.h
    Created: 19 Jul 2025 9:49:41am
    Author:  Carlos

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SynthSound.h"

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice()
    {
        adsrParams.attack = 0.1f;
        adsrParams.decay = 0.1f;
        adsrParams.sustain = 1.0f; // Sustain at full volume indefinitely
        adsrParams.release = 0.5f;
    }
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void renderNextBlock(juce::AudioBuffer< float >& outputBuffer, int startSample, int numSamples) override;
    
    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels);
    //bool shouldNoteBeRetained() const override { return true; } 

private:
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    juce::AudioBuffer<float> synthBuffer;
    

    juce::dsp::Oscillator<float> osc{ [](float x) { return x < 0.0f ? -1.0f : 1.0f; } };
    juce::dsp::Gain<float> gain;
    // return std::sin(x); } };                           Sin wave oscillator
    // return x < 0.0f ? -1.0f : 1.0f; } };               Square wave oscillator
    // return x / juce::MathConstants<float>::pi; } };    Saw wave oscillator

    bool isPrepared{ false };
};
