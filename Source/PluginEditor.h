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

    // Controles existentes
    juce::Slider minFreqSlider;
    juce::Slider maxFreqSlider;
    juce::Slider lfoSpeedSlider;
    juce::Label minLabel;
    juce::Label maxLabel;
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SirenAudioProcessorEditor)
};