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
    
    void updateParams(const juce::AudioProcessorValueTreeState& apvts);

    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels);
    void renderNextBlock(juce::AudioBuffer< float >& outputBuffer, int startSample, int numSamples) override;

private:
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    juce::AudioBuffer<float> synthBuffer;

    std::atomic<float>* lfoSpeed = nullptr;
    std::atomic<float>* minFreq = nullptr;
    std::atomic<float>* maxFreq = nullptr;


    juce::dsp::Oscillator<float> osc{ [](float x) { return std::sin(x); } };
    juce::dsp::Gain<float> gain;
    // return std::sin(x); } };                           Sin wave oscillator
    // return x < 0.0f ? -1.0f : 1.0f; } };               Square wave oscillator
    // return x / juce::MathConstants<float>::pi; } };    Saw wave oscillator


    float lfoPhase = 0.0f;
    float squarePhase = 0.0f;
    double currentSampleRate = 44100.0;
    const float lfoFreq = 115.5f;

    bool isPrepared{ false };
};
