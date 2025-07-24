/*
  ==============================================================================

    SynthSound.h
    Created: 19 Jul 2025 9:49:56am
    Author:  Carlos

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class SynthSound : public juce::SynthesiserSound
{
public:
    bool 	appliesToNote(int midiNoteNumber) override { return true; }
    bool 	appliesToChannel(int midiChannel) override { return true; }
};