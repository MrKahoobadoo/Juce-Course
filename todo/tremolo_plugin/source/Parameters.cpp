
namespace tremolo {
    namespace {
        juce::AudioParameterFloat& createModulationRateParameter(juce::AudioProcessor& processor) {
          constexpr auto versionHint = 1;
          auto parameter = std::make_unique<juce::AudioParameterFloat>(
                  juce::ParameterID{"modulation.rate", versionHint},
                  "Modulation rate",
                  juce::NormalisableRange<float>{0.1f,20.f,0.01f,0.4f},
                  5.f,
                  juce::AudioParameterFloatAttributes{}.withLabel("Hz")
                  );
          auto& parameterReference = *parameter;
          processor.addParameter(parameter.release());
          return parameterReference;
        }

        juce::AudioParameterFloat& createGainParameter(juce::AudioProcessor& processor) {
            constexpr auto versionHint = 1;
            auto parameter = std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{"gain", versionHint},
                    "Gain",
                    juce::NormalisableRange<float>{-12.0f, 12.f, 0.1f, 1.f},
                    0.f,
                    juce::AudioParameterFloatAttributes{}.withLabel("sploo")
                    .withStringFromValueFunction(
                            [](float value, int)
                            {
                                //return juce::String((std::round(value * 10.0f) / 10.0f), 1);
                                return juce::String(value, 1);
                            }
                            )
                    );
            auto& parameterReference = *parameter;
            processor.addParameter(parameter.release());
            return parameterReference;
        }

        juce::AudioParameterFloat& createDepthParameter(juce::AudioProcessor& processor) {
            constexpr auto versionHint = 1;
            auto parameter = std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{"lfo.depth", versionHint},
                    "LFO Depth",
                    juce::NormalisableRange<float>{0.f, 1.f, 0.025f, 1.f},
                    0.4f,
                    juce::AudioParameterFloatAttributes{}.withLabel("dB")
            );
            auto& parameterReference = *parameter;
            processor.addParameter(parameter.release());
            return parameterReference;
        }
    }
Parameters::Parameters(juce::AudioProcessor& processor)
// TODO: create parameters
// TODO: retrieve references to parameters
// TODO: add parameters to the processor
  : rate{createModulationRateParameter(processor)},
  gain{createGainParameter(processor)},
  depth{createDepthParameter(processor)}
{
}
}  // namespace tremolo
