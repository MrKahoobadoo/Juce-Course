#pragma once

namespace tremolo {
struct Parameters {
  explicit Parameters(juce::AudioProcessor&);

  juce::AudioParameterFloat& rate;
  juce::AudioParameterFloat& gain;
  juce::AudioParameterFloat& depth;
  juce::AudioParameterChoice& waveform;

  juce::AudioParameterBool& bypassed;

  JUCE_DECLARE_NON_COPYABLE(Parameters);
  JUCE_DECLARE_NON_MOVEABLE(Parameters);
};
}  // namespace tremolo
