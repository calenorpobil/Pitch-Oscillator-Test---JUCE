#include "PluginEditor.h"

SirenAudioProcessorEditor::SirenAudioProcessorEditor(SirenAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Configuración común para sliders
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label,
        const juce::String& text, const juce::String& paramId,
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment)
        {
            slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
            addAndMakeVisible(slider);

            label.setText(text, juce::dontSendNotification);
            label.attachToComponent(&slider, false);
            label.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(label);

            attachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
                audioProcessor.getState(), paramId, slider));
        };

    // Configurar sliders existentes
    setupSlider(minFreqSlider, minLabel, "Min Pitch (Hz)", "minFreq", minFreqAttachment);
    setupSlider(maxFreqSlider, maxLabel, "Max Pitch (Hz)", "maxFreq", maxFreqAttachment);
    setupSlider(lfoSpeedSlider, lfoSpeedLabel, "Speed (Hz)", "lfoSpeed", lfoSpeedAttachment);

    // Configurar nuevo slider de volumen
    setupSlider(gainSlider, gainLabel, "Volume", "gain", gainAttachment);
    gainSlider.setTextValueSuffix(" %");
    gainSlider.setNumDecimalPlacesToDisplay(0);
    gainSlider.textFromValueFunction = [](double value) {
        return juce::String(static_cast<int>(value * 100));
        };

    // Configurar botón de oversampling
    oversamplingToggle.setButtonText("Oversampling 4x");
    oversamplingToggle.setToggleState(true, juce::dontSendNotification);
    oversamplingToggle.setColour(juce::ToggleButton::tickColourId, juce::Colours::lightgreen);
    addAndMakeVisible(oversamplingToggle);

    oversamplingAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(
        audioProcessor.getState(), "oversampling", oversamplingToggle));

    setSize(600, 300);
}

SirenAudioProcessorEditor::~SirenAudioProcessorEditor() {}

void SirenAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawFittedText("Siren Synthesizer", getLocalBounds().removeFromTop(30),
        juce::Justification::centred, 1);
}

void SirenAudioProcessorEditor::resized()
{
    const int margin = 10;
    const int sliderWidth = 100;
    const int sliderHeight = 100;
    const int toggleHeight = 30;

    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::row;
    flexBox.flexWrap = juce::FlexBox::Wrap::wrap;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    flexBox.alignItems = juce::FlexBox::AlignItems::center;

    // Añadir controles
    flexBox.items.add(juce::FlexItem(minFreqSlider).withWidth(sliderWidth).withHeight(sliderHeight).withMargin(margin));
    flexBox.items.add(juce::FlexItem(maxFreqSlider).withWidth(sliderWidth).withHeight(sliderHeight).withMargin(margin));
    flexBox.items.add(juce::FlexItem(lfoSpeedSlider).withWidth(sliderWidth).withHeight(sliderHeight).withMargin(margin));
    flexBox.items.add(juce::FlexItem(gainSlider).withWidth(sliderWidth).withHeight(sliderHeight).withMargin(margin));

    // Diseñar controles
    flexBox.performLayout(getLocalBounds().reduced(margin).removeFromTop(250));

    // Posicionar botón de oversampling
    oversamplingToggle.setBounds(getWidth() - 150, getHeight() - 35, 140, toggleHeight);
}