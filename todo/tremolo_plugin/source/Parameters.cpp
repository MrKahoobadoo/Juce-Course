
namespace tremolo {
    namespace {

        auto& addParameterToProcessor(juce::AudioProcessor& processor, auto parameter) {
            auto& parameterReference = *parameter;
            processor.addParameter(parameter.release());
            return parameterReference;
        }

        juce::AudioParameterFloat& createModulationRateParameter(juce::AudioProcessor& processor) {
            constexpr auto versionHint = 1;
             auto parameter = std::make_unique<juce::AudioParameterFloat>(
                     juce::ParameterID{"modulation.rate", versionHint},
                  "Modulation rate",
                  juce::NormalisableRange<float>{0.1f,20.f,0.01f,0.4f},
                  5.f,
                  juce::AudioParameterFloatAttributes{}.withLabel("Hz")
                  );
            return addParameterToProcessor(processor, std::move(parameter));
        }

        juce::AudioParameterFloat& createGainParameter(juce::AudioProcessor& processor) {
            constexpr auto versionHint = 1;
            auto parameter = std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{"gain", versionHint},
                    "Gain",
                    juce::NormalisableRange<float>{-12.0f, 12.f, 0.1f, 1.f},
                    0.f,
                    juce::AudioParameterFloatAttributes{}.withLabel("dB")
                    .withStringFromValueFunction(
                            [](float value, int)
                            {
                                //return juce::String((std::round(value * 10.0f) / 10.0f), 1);
                                return juce::String(value, 1);
                            }
                            )
                    );
            return addParameterToProcessor(processor, std::move(parameter));
        }

        juce::AudioParameterFloat& createDepthParameter(juce::AudioProcessor& processor) {
            constexpr auto versionHint = 1;
            auto parameter = std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID{"lfo.depth", versionHint},
                    "LFO Depth",
                    juce::NormalisableRange<float>{0.f, 1.f, 0.025f, 1.f},
                    0.4f,
                    juce::AudioParameterFloatAttributes{}.withLabel("")
                    .withStringFromValueFunction(
                            [](float value, int)
                            {
                                return juce::String(value, 2);
                            }
                            )
            );
            return addParameterToProcessor(processor, std::move(parameter));
        }

        juce::AudioParameterBool& createBypassedParameter(juce::AudioProcessor& processor){
            constexpr auto versionHint = 1;
            auto parameter = std::make_unique<juce::AudioParameterBool>(
                    juce::ParameterID{"bypassed", versionHint},
                    "Bypass",
                    false
            );
            return addParameterToProcessor(processor, std::move(parameter));
        }

        juce::AudioParameterChoice& createWaveformParameter(juce::AudioProcessor& processor){
            constexpr auto versionHint = 1;
            auto parameter = std::make_unique<juce::AudioParameterChoice>(
                    juce::ParameterID{"modulation.waveform", versionHint},
                    "Modulation waveform",
                    juce::StringArray{"Sine", "Triangle", "Square", "Saw"},
                    0
            );
            return addParameterToProcessor(processor, std::move(parameter));
        }
    }

Parameters::Parameters(juce::AudioProcessor& processor)
  : rate{createModulationRateParameter(processor)},
  gain{createGainParameter(processor)},
  depth{createDepthParameter(processor)},
  bypassed{createBypassedParameter(processor)},
  waveform{createWaveformParameter(processor)}
{
}
}  // namespace tremolo
