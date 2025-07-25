/*
  ==============================================================================

    SynthVoice.cpp
    Created: 19 Jul 2025 9:49:41am
    Author:  Carlos

  ==============================================================================
*/

#include "SynthVoice.h"


bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<juce::SynthesiserSound*>(sound) != nullptr;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition)
{
    osc.setFrequency(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
    adsr.noteOn();
}

void SynthVoice::stopNote(float velocity, bool allowTailOff)
{
    adsr.noteOff();
}

void SynthVoice::controllerMoved(int controllerNumber, int newControllerValue)
{

}

void SynthVoice::pitchWheelMoved(int newPitchWheelValue)
{

}

void SynthVoice::updateParams(const juce::AudioProcessorValueTreeState& apvts)
{
    lfoSpeed = apvts.getRawParameterValue("lfoSpeed");
    minFreq = apvts.getRawParameterValue("minFreq");
    maxFreq = apvts.getRawParameterValue("maxFreq");
}

void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels)
{
    adsr.setSampleRate(sampleRate);

    adsr.setParameters(adsrParams);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = outputChannels;

    osc.prepare(spec);
    gain.prepare(spec);
    gain.setGainLinear(0.05f);

    isPrepared = true;
}

void SynthVoice::renderNextBlock(juce::AudioBuffer< float >& outputBuffer, int startSample, int numSamples)
{
    jassert(isPrepared);
    if (lfoSpeed != nullptr && minFreq != nullptr) {
        float lfoSpeedFloat = lfoSpeed->load();
        float minFreqFloat = minFreq->load();
        float maxFreqFloat = maxFreq->load();

        // Velocidad del cambio de pitch
        lfoPhase += 2000*lfoSpeedFloat / currentSampleRate;
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;

        // 1. Actualizar LFO (sinusoidal)
        lfoPhase += lfoFreq / currentSampleRate;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        // 2. Calcular frecuencia actual
        const float lfoValue = std::sin(2.0f * juce::MathConstants<float>::pi * lfoPhase);
        const float currentFreq = minFreqFloat + (maxFreqFloat - minFreqFloat) * (0.5f + 0.5f * lfoValue);

        // 3. Generar onda cuadrada
        squarePhase += currentFreq / currentSampleRate;
        if (squarePhase >= 1.0f) squarePhase -= 1.0f;

        //float freq = osc.getFrequency();

        osc.setFrequency(currentFreq);


    }

    juce::dsp::AudioBlock<float> audioBlock{ outputBuffer };
    osc.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
    gain.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));

    adsr.applyEnvelopeToBuffer(outputBuffer, startSample, numSamples);
}




















