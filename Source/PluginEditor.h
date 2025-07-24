/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class TapSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TapSynthAudioProcessorEditor (TapSynthAudioProcessor&);
    ~TapSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TapSynthAudioProcessor& audioProcessor;

    // Controles existentes
    juce::Slider minFreqSlider;
    juce::Slider maxFreqSlider;
    juce::Slider lfoSpeedSlider;
    juce::Label minLabel;
    juce::Label maxLabel;
<<<<<<< HEAD
    juce::Slider lfoSpeedSlider;
=======
>>>>>>> 1df56f85feed69e1db2e64bbe0578cc1b23ab7aa
    juce::Label lfoSpeedLabel;

    // Nuevos controles
    juce::Slider gainSlider;         // Control de volumen
    juce::Label gainLabel;
    juce::ToggleButton oversamplingToggle; // Botón para oversampling

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> minFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> maxFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoSpeedAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> oversamplingAttachment;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapSynthAudioProcessorEditor)
};
