#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SirenAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    SirenAudioProcessorEditor(SirenAudioProcessor&);
    ~SirenAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SirenAudioProcessor& audioProcessor;

    juce::Slider minFreqSlider;
    juce::Slider maxFreqSlider;
    juce::Label minLabel;
    juce::Label maxLabel;
    juce::Slider lfoSpeedSlider; 
    juce::Label lfoSpeedLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoSpeedAttachment;


    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> minFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> maxFreqAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SirenAudioProcessorEditor)
};