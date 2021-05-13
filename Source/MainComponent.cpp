#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() {
  // Make all child components visible
  addAndMakeVisible(playButton);
  addAndMakeVisible(playLabel);

  addAndMakeVisible(dataMenu);
  addAndMakeVisible(dataLabel);

  addAndMakeVisible(oscillatorMenu);
  addAndMakeVisible(oscillatorLabel);

  addAndMakeVisible(scaleMenu);
  addAndMakeVisible(scaleLabel);

  addAndMakeVisible(levelSlider);
  addAndMakeVisible(levelLabel);

  addAndMakeVisible(minPitchSlider);
  addAndMakeVisible(minPitchLabel);

  addAndMakeVisible(maxPitchSlider);
  addAndMakeVisible(maxPitchLabel);

  addAndMakeVisible(playbackBpmSlider);
  addAndMakeVisible(playbackBpmLabel);

  addAndMakeVisible(minMaxUnitButton);

  // Add listeners to child components
  playButton.addListener(this);
  dataMenu.addListener(this);
  oscillatorMenu.addListener(this);
  scaleMenu.addListener(this);
  levelSlider.addListener(this);
  minPitchSlider.addListener(this);
  maxPitchSlider.addListener(this);
  playbackBpmSlider.addListener(this);

  // Add items to combo box components
  oscillatorMenu.addItemList({"Sine", "Square", "Triangle", "Saw"}, kSine);
  scaleMenu.addItemList(
      {"Frequency", "Chromatic", "Diatonic", "Pentatonic", "Whole Tone"},
      kFrequency);

  // Initialize level slider
  levelSlider.setRange(kMinLevel, kMaxLevel);
  levelSlider.setSliderStyle(Slider::LinearHorizontal);
  levelSlider.setTextBoxStyle(Slider::TextBoxLeft, false, 90, 22);
  levelSlider.setValue(level);

  // Initialize minimum pitch slider
  minPitchSlider.setRange(kMinMidiPitch, kMaxMidiPitch, 1);
  minPitchSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
  minPitchSlider.setValue(minMidiPitch);

  // Initialize maximum pitch slider
  maxPitchSlider.setRange(kMinMidiPitch, kMaxMidiPitch, 1);
  maxPitchSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
  maxPitchSlider.setValue(maxMidiPitch);

  // Initialize BPM slider
  playbackBpmSlider.setRange(kMinBpm, kMaxBpm, 1);
  playbackBpmSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
  playbackBpmSlider.setValue(playbackBpm);

  // Initialize play button
  playButton.setEnabled(false);
  drawPlayButton(playButton, true);

  // Initialize MIDI to frequency lookup table
  for (int i = kMinMidiPitch; i <= kMaxMidiPitch; i++) {
    double freq = convertMidiToFreq(i);
    midiToFreqTable.add(freq);
  }

  // Make sure you set the size of the component after
  // you add any child components.
  setSize(800, 600);

  // Some platforms require permissions to open input channels so request that
  // here
  if (juce::RuntimePermissions::isRequired(
          juce::RuntimePermissions::recordAudio) &&
      !juce::RuntimePermissions::isGranted(
          juce::RuntimePermissions::recordAudio)) {
    juce::RuntimePermissions::request(
        juce::RuntimePermissions::recordAudio,
        [&](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
  } else {
    // Specify the number of input and output channels that we want to open
    setAudioChannels(2, 2);
  }

  audioSourcePlayer.setSource(nullptr);
  setVisible(true);
}

MainComponent::~MainComponent() {
  // This shuts down the audio device and clears the audio source.
  shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected,
                                  double sampleRate) {
  // This function will be called when the audio device is started, or when
  // its settings (i.e. sample rate, block size, etc) are changed.

  // You can use this function to initialise any resources you might need,
  // but be careful - it will be called on the audio thread, not the GUI thread.

  // For more details, see the help for AudioProcessor::prepareToPlay()
  srate = sampleRate;
  phase = 0;
  phaseDelta = currentFreq / srate;
  time = 0;
}

void MainComponent::getNextAudioBlock(
    const juce::AudioSourceChannelInfo& bufferToFill) {
  // Your audio-processing code goes here!

  // For more details, see the help for AudioProcessor::getNextAudioBlock()

  // Right now we are not producing any data, in which case we need to clear the
  // buffer (to prevent the output of random noise)
  bufferToFill.clearActiveBufferRegion();

  if (!isPlaying()) return;

  // If we've run out of notes, we can stop playing
  if (amountsToPlay.isEmpty()) {
    audioSourcePlayer.setSource(nullptr);
  }

  // Find note duration, in samples
  double beatsPerSecond = playbackBpm / 60.0;
  double noteDurationInSamples = srate / beatsPerSecond;

  switch (oscillatorId) {
    case kSine:
      generateSine(bufferToFill);
      break;
    case kSquare:
      generateSquare(bufferToFill);
      break;
    case kTriangle:
      generateTriangle(bufferToFill);
      break;
    case kSaw:
      generateSaw(bufferToFill);
      break;
    case kNoOscilator:
      break;
  }
}

void MainComponent::releaseResources() {
  // This will be called when the audio device stops, or when it is being
  // restarted due to a setting change.

  // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  // You can add your drawing code here!
}

void MainComponent::resized() {
  const int COL_HEIGHT = 30;
  const int PADDING = 8;
  const int SLIGHT_PADDING = 4;
  const int MENU_WIDTH = 118;
  const int LABEL_WIDTH = 65;
  const int SLIDER_WIDTH = 338;

  auto componentBounds = getLocalBounds();
  componentBounds.reduce(PADDING, PADDING);

  auto firstRow = componentBounds.removeFromTop(COL_HEIGHT);
  playLabel.setBounds(firstRow.removeFromLeft(LABEL_WIDTH));
  playButton.setBounds(firstRow.removeFromLeft(COL_HEIGHT));
  levelSlider.setBounds(firstRow.removeFromRight(SLIDER_WIDTH));
  levelLabel.setBounds(firstRow.removeFromRight(LABEL_WIDTH));

  componentBounds.removeFromTop(
      PADDING);  // padding between first and second row

  auto secondRow = componentBounds.removeFromTop(COL_HEIGHT);
  dataLabel.setBounds(secondRow.removeFromLeft(LABEL_WIDTH));
  dataMenu.setBounds(secondRow.removeFromLeft(MENU_WIDTH));
  playbackBpmSlider.setBounds(secondRow.removeFromRight(SLIDER_WIDTH));
  playbackBpmLabel.setBounds(secondRow.removeFromRight(LABEL_WIDTH));

  auto bottomRow = componentBounds.removeFromBottom(COL_HEIGHT);
  auto thirdRow = componentBounds.removeFromBottom(COL_HEIGHT);
  oscillatorLabel.setBounds(thirdRow.removeFromLeft(LABEL_WIDTH));
  oscillatorMenu.setBounds(thirdRow.removeFromLeft(MENU_WIDTH));
  thirdRow.removeFromLeft(SLIGHT_PADDING);
  scaleLabel.setBounds(thirdRow.removeFromLeft(LABEL_WIDTH));
  scaleMenu.setBounds(thirdRow.removeFromLeft(MENU_WIDTH));
  thirdRow.removeFromLeft(SLIGHT_PADDING);

  minPitchSlider.setBounds(thirdRow.removeFromRight(SLIDER_WIDTH));
  minPitchLabel.setBounds(thirdRow.removeFromRight(LABEL_WIDTH));
  maxPitchSlider.setBounds(bottomRow.removeFromRight(SLIDER_WIDTH));
  maxPitchLabel.setBounds(bottomRow.removeFromRight(LABEL_WIDTH));
}

void MainComponent::sliderValueChanged(Slider* slider) {
  if (slider == &levelSlider) {
    level = slider->getValue();
  } else if (slider == &minPitchSlider) {
    minMidiPitch = slider->getValue();
    // FIXME: temporarily using min midi pitch to test oscillators
    currentFreq = midiToFreqTable[minMidiPitch];
    phaseDelta = currentFreq / srate;
  } else if (slider == &maxPitchSlider) {
    maxMidiPitch = slider->getValue();
  } else if (slider == &playbackBpmSlider) {
    playbackBpm = slider->getValue();
  }
}

void MainComponent::comboBoxChanged(ComboBox* menu) {
  int index = menu->getSelectedItemIndex();
  if (menu == &oscillatorMenu) {
    auto nextOsc = OscillatorId(kNoOscilator + index + 1);
    oscillatorId = nextOsc;
    if (oscillatorId != kNoOscilator) {
      playButton.setEnabled(true);
    }
  } else if (menu == &scaleMenu) {
    auto nextScale = ScaleId(kNoScale + index + 1);
    scaleId = nextScale;
  } else if (menu == &dataMenu) {
  }
}

void MainComponent::buttonClicked(Button* button) {
  if (button == &playButton) {
    if (isPlaying()) {
      audioSourcePlayer.setSource(nullptr);
      drawPlayButton(playButton, true);
      amountsToPlay.clear();
    } else {
      audioSourcePlayer.setSource(this);
      drawPlayButton(playButton, false);
      amountsToPlay = generateRandomAmounts(0, 50, 100, 25);
    }
  }
}

bool MainComponent::isPlaying() {
  return audioSourcePlayer.getCurrentSource() != nullptr;
}

void MainComponent::drawPlayButton(juce::DrawableButton& button,
                                   bool showPlay) {
  juce::Path path;
  if (showPlay) {
    // draw the triangle
    path.addTriangle({0, 0}, {0, 100}, {100, 50});
  } else {
    // draw the two bars
    int width = 42;
    path.addRectangle(0, 100, width, 100);
    path.addRectangle(100 - width, 100, width, 100);
  }
  juce::DrawablePath drawable;
  drawable.setPath(path);
  juce::FillType fill(Colours::white);
  drawable.setFill(fill);
  button.setImages(&drawable);
}

double MainComponent::phasor() {
  double p = phase;
  phase = std::fmod(phase + phaseDelta, 1.0);
  return p;
}

void MainComponent::generateSine(const AudioSourceChannelInfo& bufferToFill) {
  double startingPhase = phase;
  for (int channel = 0; channel < bufferToFill.buffer->getNumChannels();
       channel++) {
    phase = startingPhase;
    auto channelData =
        bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

    for (int i = 0; i < bufferToFill.numSamples; i++) {
      channelData[i] = level * std::sin(phasor() * TwoPi);
    }
  }
}

void MainComponent::generateSquare(const AudioSourceChannelInfo& bufferToFill) {
  double startingPhase = phase;
  for (int channel = 0; channel < bufferToFill.buffer->getNumChannels();
       channel++) {
    phase = startingPhase;
    auto channelData =
        bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

    for (int i = 0; i < bufferToFill.numSamples; i++) {
      double currentPhasor = phasor();
      if (currentPhasor <= 0.5) {
        channelData[i] = -1 * level;
      } else {
        channelData[i] = 1 * level;
      }
    }
  }
}

void MainComponent::generateTriangle(
    const AudioSourceChannelInfo& bufferToFill) {
  double startingPhase = phase;
  for (int channel = 0; channel < bufferToFill.buffer->getNumChannels();
       channel++) {
    phase = startingPhase;
    auto channelData =
        bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

    for (int i = 0; i < bufferToFill.numSamples; i++) {
      double currentPhasor = phasor();
      if (currentPhasor <= 0.5) {
        // Range [0, 0.5] -> [0, 2] -> [-1, 1]
        channelData[i] = level * (4 * currentPhasor - 1);
      } else {
        // Range [0.5, 1] -> [-0.5, -1] -> [0, -0.5] -> [0, -2] -> [1, -1]
        channelData[i] = level * (4 * (-currentPhasor + 0.5) + 1);
      }
    }
  }
}

void MainComponent::generateSaw(const AudioSourceChannelInfo& bufferToFill) {
  double startingPhase = phase;
  for (int channel = 0; channel < bufferToFill.buffer->getNumChannels();
       channel++) {
    phase = startingPhase;
    auto channelData =
        bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

    for (int i = 0; i < bufferToFill.numSamples; i++) {
      channelData[i] = level * (2 * phasor() - 1);
    }
  }
}

float MainComponent::getRandomSample() {
  return random.nextFloat() * 2.0f - 1.0f;
}

float MainComponent::getRandomSample(float amp) {
  return getRandomSample() * amp;
}

int MainComponent::convertFreqToMidi(double freq) {
  // Taken from: https://www.music.mcgill.ca/~gary/307/week1/node28.html
  int midi = (int)((12 * log(freq / 220.0) / log(2.0)) + 57.01);
  return midi;
}

double MainComponent::convertMidiToFreq(int midi) {
  // Taken from: https://www.music.mcgill.ca/~gary/307/week1/node28.html
  double freq = 440.0 * std::pow(2, (double)(midi - 69) / 12.0);
  return freq;
}

juce::Array<double> MainComponent::generateRandomAmounts(double start, double end,
                                                      double range,
                                                      int length) {
  juce::Array<double> arr;
  for (int i = 0; i < length; i++) {
    double a = random.nextDouble() * range;
    double b = random.nextDouble() * 2 - 1;
    double c = random.nextDouble() * (range / 4);
    double d = random.nextDouble() * 2 - 1;
    double x = (end - start) * (i / length);
    double amount = generateRandomAmount(a, b, c, d, x);
    arr.add(amount);
  }
  return arr;
}

double MainComponent::generateRandomAmount(double a, double b, double c,
    double d, double x) {
  return a * cos(b * x) + c * sin(d * x) + a + c;
}
