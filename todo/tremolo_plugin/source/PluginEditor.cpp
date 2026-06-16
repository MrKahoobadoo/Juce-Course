namespace tremolo {
PluginEditor::PluginEditor(PluginProcessor& p) : AudioProcessorEditor(&p) {

  //setResizable (true, true);
  //setResizeLimits (400, 200, 1000, 500);

  //const float ratio = 2.f;
  //getConstrainer ()->setFixedAspectRatio (ratio);

  background.setImage(juce::ImageCache::getFromMemory(
      assets::Background_png, assets::Background_pngSize));

  logo.setImage(
      juce::ImageCache::getFromMemory(assets::Chicken_Tremolo_Logo_png, assets::Chicken_Tremolo_Logo_pngSize));

  addAndMakeVisible(background);
  addAndMakeVisible(logo);
  addAndMakeVisible(lfoVisualizer);

  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(540, 270);
}

void PluginEditor::resized() {
  const auto bounds = getLocalBounds();

  //cout << "x: " << bounds.getX() << ", y: " << bounds.getY() << ", width: " << bounds.getWidth() << ", height: " <<
  //bounds.getHeight() << "\n";

  background.setBounds(bounds);

  int logoBorder = 10;
  int logoHeight = 32;
  int logoWidth = logoHeight*10;

  logo.setBounds({logoBorder, logoBorder, logoWidth, logoHeight});

  lfoVisualizer.setBounds({18, 149, 504, 92});
}

void LfoVisualizer::resized () {
  sine.clear();
  //setBounds({18, 149, 504, 92});

  const auto overLap = 5.f;
  //const auto strokeWidth = 4.f;
  const auto halfHeight = getHeight() / 2;
  const auto amplitude = halfHeight - strokeWidth / 2;

  sine.startNewSubPath(-overLap, halfHeight + amplitude * std::sin(0.1f * -overLap));

  for (const auto x : std::views::iota(1, getWidth() + 2*overLap)) {
    sine.lineTo(x, std::sin(0.1f * x) * amplitude + halfHeight);
  }

  //sine.clear();
}
}  // namespace tremolo
