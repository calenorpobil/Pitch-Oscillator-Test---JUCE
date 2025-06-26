#pragma once
#include <JuceHeader.h>

class SirenAudioProcessor : public juce::AudioProcessor
{
public:
    SirenAudioProcessor();
    ~SirenAudioProcessor() override = default;

    // Métodos obligatorios de AudioProcessor
    const juce::String getName() const override { return "SquareSiren"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getState() { return *apvts; }

private:
    std::unique_ptr<juce::AudioProcessorValueTreeState> apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    float lfoPhase = 0.0f;
    float squarePhase = 0.0f;
    double currentSampleRate = 44100.0;
    const float lfoFreq = 0.5f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SirenAudioProcessor)
};