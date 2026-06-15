namespace tremolo {
namespace {
    struct SerializableParameters {
        float rate;
        float gain;
        float depth;
        juce::String waveform;
        bool bypassed;

        static constexpr auto marshallingVersion = 1;

        template <typename Archive, typename T>
        static void serialise(Archive& archive, T& t) {
            using namespace juce;

            if (archive.getVersion() != 1) {
                return;
            }

            std::string pluginName = TREMOLO_PLUGIN_NAME;

            archive(named("pluginName", pluginName));

            if (pluginName != TREMOLO_PLUGIN_NAME) {
                return;
            }

            archive(named("modulationRateHz", t.rate),
                    //named("gain", t.gain),
                    //named("depth", t.depth),
                    named("bypassed", t.bypassed),
                    named("modulationWaveform", t.waveform));
        }
    };

    SerializableParameters from(const tremolo::Parameters& parameters) {
      return {
              .rate = parameters.rate.get(),
              .gain = parameters.gain.get(),
              .depth = parameters.depth.get(),
              .waveform = parameters.waveform.getCurrentChoiceName(),
              .bypassed = parameters.bypassed.get()
      };
    }
}

void JsonSerializer::serialize(const Parameters& parameters,
                               juce::OutputStream& output) {
  juce::ignoreUnused(parameters, output);
  const auto parametersToSerialize = from(parameters);

  const auto json = juce::ToVar::convert(parametersToSerialize);

  if (!json.has_value()) {
    return;
  }

  juce::JSON::writeToStream(output, *json, juce::JSON::FormatOptions{}
  .withSpacing(juce::JSON::Spacing::multiLine)
  .withMaxDecimalPlaces(2));

}

juce::Result JsonSerializer::deserialize(juce::InputStream& input,
                                         Parameters& parameters) {
  juce::var parsedResult;
  const auto result = juce::JSON::parse(input.readEntireStreamAsString(),
                                        parsedResult);

  if (result.failed()) {
      return result;
  }

  const auto parsedParameters = juce::FromVar::convert<SerializableParameters>
          (parsedResult);

  if (!parsedParameters.has_value()) {
      return juce::Result::fail("failed to parse parameters from JSOn representation");
  }

  //if (modulationWaveformIndex != "Sine" || modulationWaveformIndex != "Sine" )

  const auto modulationWaveformIndex = parameters.waveform.choices.indexOf(
          parsedParameters->waveform);

  if (modulationWaveformIndex != juce::toUnderlyingType(Tremolo::LfoWaveform::sine) &&
        modulationWaveformIndex != juce::toUnderlyingType(Tremolo::LfoWaveform::triangle) &&
        modulationWaveformIndex != juce::toUnderlyingType(Tremolo::LfoWaveform::square) &&
        modulationWaveformIndex != juce::toUnderlyingType(Tremolo::LfoWaveform::saw)) {
      return juce::Result::fail("failed due to invalid waveform");
  }
  
  parameters.waveform = modulationWaveformIndex;
  parameters.rate = parsedParameters->rate;
  parameters.gain = parsedParameters->gain;
  parameters.depth = parsedParameters->depth;
  parameters.bypassed = parsedParameters->bypassed;

  return juce::Result::ok();
}
}  // namespace tremolo
