#pragma once

namespace tremolo {
    enum class ApplySmoothing { no, yes };

class Tremolo {
public:
    enum class LfoWaveform : size_t {
        sine = 0, triangle = 1, square = 2, saw = 3
    };

  Tremolo() {
    setModulationRateHz(5.f, ApplySmoothing::no),
    currentGain = 1.f, currentDepth = 0.4f;
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

  void setGainToSet(float dB) noexcept {
      globalGainToSet = juce::Decibels::decibelsToGain(dB);
  }

  void setDepthToSet(float depth) noexcept {
      globalDepthToSet = depth;
  }

//  void hardSetParams(float gain, float depth) {
//    gainValue = gain;
//    currentGain = gain;
//    depthValue = depth;
//    currentDepth = depth;
//  }

  void setLfoWaveform(LfoWaveform waveform, ApplySmoothing applySmoothing = ApplySmoothing::yes) {
    jassert(waveform == LfoWaveform::sine || waveform == LfoWaveform::triangle || waveform == LfoWaveform::square || waveform == LfoWaveform::saw);

    lfoToSet = waveform;

    if (applySmoothing == ApplySmoothing::no) {
      currentLfo = waveform;
    }
  }

  void process(juce::AudioBuffer<float>& buffer) noexcept {
    updateLfoWaveform();


    updateParameterSmoothing(gainTransitionSmoother, currentGain, globalGainToSet);
    updateParameterSmoothing(depthTransitionSmoother, currentDepth, globalDepthToSet);

    // for each frame
    for (const auto frameIndex : std::views::iota(0, buffer.getNumSamples())) {
      //const auto bug = lfo.processSample(0.f);

      const auto lfoValue = getNextLfoValue();
      //std::cout << lfoValue << std::endl;
      gainValue = gainTransitionSmoother.getNextValue();
      depthValue = depthTransitionSmoother.getNextValue();

      // starts at gain = 1, then oscillates above and below
      const auto modulationValue = lfoValue * depthValue + 1.f;

      // for cosine wave and ensuring max gain = 1
      // const auto modulationValue = 1.f - (lfoValue + 1.f) * modulationDepth / 2.f;

      // std::cout << lfoValue << std::endl;

      // for each channel sample in the frame
      for (const auto channelIndex :
           std::views::iota(0, buffer.getNumChannels())) {
        // get the input sample
        const auto inputSample = buffer.getSample(channelIndex, frameIndex);

        const auto outputSample = inputSample * modulationValue * gainValue;

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
    //const auto ft = phase / juce::MathConstants<float>::twoPi;
    float val = 0.f;
    const float slideDur = 0.05f;
    if (phase <= -slideDur) {
      val = 0.f;
    } else if (phase <= 0) {
      val = (slideDur + phase) / slideDur;
    } else if (phase >= (juce::MathConstants<float>::pi - slideDur)) {
      val = (juce::MathConstants<float>::pi - phase) / slideDur;
    } else {
      val = 1.f;
    }

    val = 2.f * (val - 0.5f);
    //std::cout << val << std::endl;
    return val;
  }

  static float saw(float phase) {
    float val = 0.f;
    const float slideDur = 0.05f;
    if (phase <= juce::MathConstants<float>::pi - slideDur) {
      val = phase / juce::MathConstants<float>::pi;
    } else {
      val = (((juce::MathConstants<float>::pi - phase) / slideDur) - 0.5f) * 2.f;
    }

    val = (val - 0.5f)*2.f;

    //std::cout << val << std::endl;
    return val;
    //return -phase / juce::MathConstants<float>::twoPi * 2.f;

  }

  void updateLfoWaveform() {
    if (currentLfo != lfoToSet) {
      lfoTransitionSmoother.setCurrentAndTargetValue(getNextLfoValue());

      currentLfo = lfoToSet;

      lfoTransitionSmoother.setTargetValue(getNextLfoValue());
    }
  }

  void updateParameterSmoothing(juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>& smoothedParameter, float& current, float& target) {
    if (abs(current - target) > 0.001f) {
      smoothedParameter.setCurrentAndTargetValue(current);

      current = target;

      smoothedParameter.setTargetValue(target);
    }
  }

  std::array<juce::dsp::Oscillator<float>, 4u> lfos
  {
    juce::dsp::Oscillator<float>{[](auto phase){ return std::sin(phase); }},
    juce::dsp::Oscillator<float>{triangle},
    juce::dsp::Oscillator<float>{square},
    juce::dsp::Oscillator<float>{saw}
  };

  LfoWaveform currentLfo = LfoWaveform::sine;
  LfoWaveform lfoToSet = currentLfo;

  float gainValue = 1.f;
  float currentGain, globalGainToSet = gainValue;

  float depthValue = 0.4f;
  float currentDepth, globalDepthToSet = depthValue;

  juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> lfoTransitionSmoother{0.f};
  juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> gainTransitionSmoother{1.f};
  juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> depthTransitionSmoother{0.4f};

};
}  // namespace tremolo









