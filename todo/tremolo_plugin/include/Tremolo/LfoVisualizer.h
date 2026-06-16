namespace tremolo{
class LfoVisualizer : public juce::Component {
public:
    void paint (juce::Graphics& g) override {
        g.setColour(juce::Colours::orange);
        g.strokePath(sine, juce::PathStrokeType{strokeWidth});

    }

    juce::Path sine;
    const float strokeWidth = 4.f;

    void resized () override;
};
}