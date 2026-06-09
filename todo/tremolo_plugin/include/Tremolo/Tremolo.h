#pragma once

namespace tremolo {
    enum class ApplySmoothing { no, yes };

class Tremolo {
public:
    enum class LfoWaveform : size_t {
        sine = 0, triangle = 1, square = 2, saw = 3
    };

  Tremolo() {
    setModulationRateHz(5.f, ApplySmoothing::no);
    setGlobalGaindB(1.f);
  }

  void prepare(double sampleRate, int expectedMaxFramesPerBlock) {
    const juce::dsp::ProcessSpec processSpec {
      .sampleRate = sampleRate,
      .maximumBlockSize = static_cast<juce::uint32>(expectedMaxFramesPerBlock),
      .numChannels = 1u,
    };
    for (auto& lfo : lfos) {
      lfo.prepare(processSpec);
    }

    lfoTransitionSmoother.reset(sampleRate, 0.025);
    gainTransitionSmoother.reset(sampleRate, 0.025);
  }

  void setModulationRateHz(float rateHz, ApplySmoothing applySmoothing = ApplySmoothing::yes) noexcept {
    const auto force = applySmoothing == ApplySmoothing::no;
    for (auto& lfo : lfos) {
      lfo.setFrequency(rateHz, force);
    }
  }

  void setGlobalGaindB(float dB) noexcept {
      globalGain = juce::Decibels::decibelsToGain(dB);
  }


  void setLfoWaveform(LfoWaveform waveform, ApplySmoothing applySmoothing = ApplySmoothing::yes) {
    jassert(waveform == LfoWaveform::sine || waveform == LfoWaveform::triangle || waveform == LfoWaveform::square || waveform == LfoWaveform::saw);

    lfoToSet = waveform;

    if (applySmoothing == ApplySmoothing::no) {
      currentLfo = waveform;
    }
  }

  void process(juce::AudioBuffer<float>& buffer) noexcept {
    updateLfoWaveform();
    //updateGlobalGain();

    // for each frame
    for (const auto frameIndex : std::views::iota(0, buffer.getNumSamples())) {
      //const auto bug = lfo.processSample(0.f);

      const float lfoValue = getNextLfoValue();

      constexpr auto modulationDepth = 0.4f;

      // starts at gain = 1, then oscillates above and below
      const auto modulationValue = lfoValue * modulationDepth + 1.f;

      // for cosine wave and ensuring max gain = 1
      // const auto modulationValue = 1.f - (lfoValue + 1.f) * modulationDepth / 2.f;

      // std::cout << lfoValue << std::endl;

      // for each channel sample in the frame
      for (const auto channelIndex :
           std::views::iota(0, buffer.getNumChannels())) {
        // get the input sample
        const auto inputSample = buffer.getSample(channelIndex, frameIndex);

        const auto outputSample = inputSample * modulationValue * globalGain;//gainTransitionSmoother.getNextValue();

        // set the output sample
        buffer.setSample(channelIndex, frameIndex, outputSample);
      }
    }
  }

    void reset() noexcept {
    for (auto& lfo : lfos) {
      lfo.reset();
    }
  }

private:
  // You should put class members and private functions here

  float getNextLfoValue() {
    if (lfoTransitionSmoother.isSmoothing()) {
      return lfoTransitionSmoother.getNextValue();
    }

    return lfos[juce::toUnderlyingType(currentLfo)].processSample(0.f);
  }

  static float triangle(float phase) {
    const auto ft = phase / juce::MathConstants<float>::twoPi;
    return 4.f * std::abs(ft - std::floor(ft + 0.5f)) - 1.f;
  }

  static float square(float phase) {
    const auto bug = std::sin(phase);
    if (bug > 0) {
      return 1.f;
    } else {
      return -1.f;
    }
  }

  static float saw(float phase) {
    return -phase / juce::MathConstants<float>::twoPi * 2.f;
  }

  void updateLfoWaveform() {
      if (currentLfo != lfoToSet) {
        lfoTransitionSmoother.setCurrentAndTargetValue(getNextLfoValue());

        currentLfo = lfoToSet;

        lfoTransitionSmoother.setTargetValue(getNextLfoValue());
      }
  }

//  void updateGlobalGain() {
//    if (abs(globalGain - globalGainToSet) > 0.001) {
//      gainTransitionSmoother.setCurrentAndTargetValue(globalGain);
//
//      gainTransitionSmoother.setTargetValue(globalGainToSet);
//    } else {
//      globalGain = globalGainToSet;
//    }
//
//  }

  std::array<juce::dsp::Oscillator<float>, 4u> lfos
  {
    juce::dsp::Oscillator<float>{[](auto phase){ return std::sin(phase); }},
    juce::dsp::Oscillator<float>{triangle},
    juce::dsp::Oscillator<float>{square},
    juce::dsp::Oscillator<float>{saw}

  };

  LfoWaveform currentLfo = LfoWaveform::sine;
  LfoWaveform lfoToSet = currentLfo;

  float globalGain = 1.f;
  float globalGainToSet = globalGain;

  juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> lfoTransitionSmoother{0.f};
  juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> gainTransitionSmoother{0.f};

};
}  // namespace tremolo









