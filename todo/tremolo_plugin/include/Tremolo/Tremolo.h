#pragma once

namespace tremolo {
class Tremolo {
public:
    enum class LfoWaveform : size_t {
        sine = 0, triangle = 1, square = 2, saw = 3
    };
  Tremolo() {
    for (auto& lfo : lfos) {
      lfo.setFrequency(5.f /* hz */, true);
    }
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
  }

  void setLfoWaveform(LfoWaveform waveform) {
    jassert(waveform == LfoWaveform::sine || waveform == LfoWaveform::triangle || waveform == LfoWaveform::square || waveform == LfoWaveform::saw);

    lfoToSet = waveform;
  }

  void process(juce::AudioBuffer<float>& buffer) noexcept {
    updateLfoWaveform();
    // for each frame
    for (const auto frameIndex : std::views::iota(0, buffer.getNumSamples())) {
      //const auto bug = lfo.processSample(0.f);

      float lfoValue = getNextLfoValue();

      float lfoSquare = 0.f;
      if (lfoValue > 0) {
        lfoSquare = 1.f;
      } else {
        lfoSquare = -1.f;
      }
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

        const auto outputSample = inputSample * modulationValue;

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
      currentLfo = lfoToSet;
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
};
}  // namespace tremolo
