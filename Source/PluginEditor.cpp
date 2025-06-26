#include "PluginEditor.h"

SirenAudioProcessorEditor::SirenAudioProcessorEditor(SirenAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Configuración del slider de velocidad
    lfoSpeedSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    lfoSpeedSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    lfoSpeedSlider.setRange(0.1, 5.0, 0.01);  // Coincide con el rango del parámetro
    addAndMakeVisible(lfoSpeedSlider);

    lfoSpeedLabel.setText("Speed (Hz)", juce::dontSendNotification);
    lfoSpeedLabel.attachToComponent(&lfoSpeedSlider, false);
    lfoSpeedLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lfoSpeedLabel);


    // Configurar sliders
    minFreqSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    minFreqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(minFreqSlider);

    maxFreqSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    maxFreqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(maxFreqSlider);

    // Configurar labels
    minLabel.setText("Minimum Pitch (Hz)", juce::dontSendNotification);
    minLabel.attachToComponent(&minFreqSlider, false);
    minLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(minLabel);

    maxLabel.setText("Maximum Pitch (Hz)", juce::dontSendNotification);
    maxLabel.attachToComponent(&maxFreqSlider, false);
    maxLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(maxLabel);

    // Vincular sliders a partros (CORRECCIN CLAVE)
    minFreqAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getState(), "minFreq", minFreqSlider));

    maxFreqAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getState(), "maxFreq", maxFreqSlider));

    lfoSpeedAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getState(),
        "lfoSpeed", lfoSpeedSlider));

    setSize(400, 250);
}

SirenAudioProcessorEditor::~SirenAudioProcessorEditor() {}

void SirenAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void SirenAudioProcessorEditor::resized()
{
    const int margin = 20;
    const int sliderWidth = 150;
    const int sliderHeight = 150;

    minFreqSlider.setBounds(margin, 40, sliderWidth, sliderHeight);
    maxFreqSlider.setBounds(getWidth() - sliderWidth - margin, 40, sliderWidth, sliderHeight);
    lfoSpeedSlider.setBounds(getWidth() / 2 - sliderWidth / 2, 40, sliderWidth, sliderWidth);
}