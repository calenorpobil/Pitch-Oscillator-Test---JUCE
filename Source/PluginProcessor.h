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

    float calculateSmoothedFrequency(int sample, int totalSamples);

    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getState() { return *apvts; }

private:
    std::unique_ptr<juce::AudioProcessorValueTreeState> apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    // Osciladores DSP
    juce::dsp::Oscillator<float> lfoOscillator;  // Oscilador LFO
    juce::dsp::Oscillator<float> mainOscillator; // Oscilador principal

    // Sistema de oversampling
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

    float lfoPhase = 0.0f;
    float oscillatorPhase = 0.0f;
    double currentSampleRate = 44100.0;
    
    // Parámetros suavizados
    juce::SmoothedValue<float> smoothedMinFreq{ 500.0f };
    juce::SmoothedValue<float> smoothedMaxFreq{ 500.0f };
    juce::SmoothedValue<float> smoothedLfoSpeed{ 0.5f };
    juce::SmoothedValue<float> smoothedGain{ 0.75f };  // Volumen al 75% por defecto

    // Filtro anti-aliasing
    juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>> antiAliasingFilter;


    bool oversamplingEnabled = true;  // Habilitar/deshabilitar oversampling

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SirenAudioProcessor)
};