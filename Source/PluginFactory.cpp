#include "PluginProcessor.h"
#include "PluginEditor.h"

// Función obligatoria para que JUCE cree la instancia del plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SirenAudioProcessor();  // Asegúrate que coincide con tu clase
}