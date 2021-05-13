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
    return;
  }

  // Find note duration, in samples
  double beatsPerSecond = playbackBpm / 60.0;
  int noteDurationInSamples = std::ceil(srate / beatsPerSecond);

  switch (oscillatorId) {
    case kSine:
      generateSine(bufferToFill, noteDurationInSamples);
      break;
    case kSquare:
      generateSquare(bufferToFill, noteDurationInSamples);
      break;
    case kTriangle:
      generateTriangle(bufferToFill, noteDurationInSamples);
      break;
    case kSaw:
      generateSaw(bufferToFill, noteDurationInSamples);
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
  if (!isPlaying()) {
    drawPlayButton(playButton, true);

    // Enable sliders when not playing
    // FIXME: is there a way to do this without checking every frame?
    levelSlider.setEnabled(true);
    minPitchSlider.setEnabled(true);
    maxPitchSlider.setEnabled(true);
    playbackBpmSlider.setEnabled(true);
    oscillatorMenu.setEnabled(true);
    dataMenu.setEnabled(true);
    scaleMenu.setEnabled(true);
  } else {
    drawPlayButton(playButton, false);
  }
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
      // Stop playback
      audioSourcePlayer.setSource(nullptr);
      amountsToPlay.clear();
    } else {
      // Generate notes to play
      amountsToPlay = generateRandomAmounts(0, 0.5, 100, 25);
      convertAmountsToNotes(amountsToPlay);

      // Set frequency
      currentFreq = midiToFreqTable[amountsToPlay.begin()->first];
      phaseDelta = currentFreq / srate;

      // Disable sliders and menus
      levelSlider.setEnabled(false);
      minPitchSlider.setEnabled(false);
      maxPitchSlider.setEnabled(false);
      playbackBpmSlider.setEnabled(false);
      oscillatorMenu.setEnabled(false);
      dataMenu.setEnabled(false);
      scaleMenu.setEnabled(false);

      // Generate audio
      audioSourcePlayer.setSource(this);
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

void MainComponent::generateSine(const AudioSourceChannelInfo& bufferToFill,
                                 int noteDuration) {
  double startingPhase = phase;
  for (int channel = 0; channel < bufferToFill.buffer->getNumChannels();
       channel++) {
    phase = startingPhase;
    auto channelData =
        bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

    for (int i = 0; i < bufferToFill.numSamples; i++) {
      channelData[i] = level * std::sin(phasor() * TwoPi);
      bool playbackIsFinished = decrementNoteDurations();
      if (playbackIsFinished) return;
    }
  }
}

void MainComponent::generateSquare(const AudioSourceChannelInfo& bufferToFill,
                                   int noteDuration) {
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
      bool playbackIsFinished = decrementNoteDurations();
      if (playbackIsFinished) return;
    }
  }
}

void MainComponent::generateTriangle(const AudioSourceChannelInfo& bufferToFill,
                                     int noteDuration) {
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
      bool playbackIsFinished = decrementNoteDurations();
      if (playbackIsFinished) return;
    }
  }
}

void MainComponent::generateSaw(const AudioSourceChannelInfo& bufferToFill,
                                int noteDuration) {
  double startingPhase = phase;
  for (int channel = 0; channel < bufferToFill.buffer->getNumChannels();
       channel++) {
    phase = startingPhase;
    auto channelData =
        bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

    for (int i = 0; i < bufferToFill.numSamples; i++) {
      channelData[i] = level * (2 * phasor() - 1);
      bool playbackIsFinished = decrementNoteDurations();
      if (playbackIsFinished) return;
    }
  }
}

bool MainComponent::decrementNoteDurations() {
  // Decrement sample
  amountsToPlay.begin()->second--;
  // If entire note duration has been played,
  if (amountsToPlay.begin()->second == 0) {
    // Remove that note and move on to next
    amountsToPlay.removeAndReturn(0);
    // Change phaseDelta to match new note
    if (amountsToPlay.isEmpty()) {
      return true;
    }
    currentFreq = midiToFreqTable[amountsToPlay.begin()->first];
    phaseDelta = currentFreq / srate;
  }
  return false;
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

juce::Array<std::pair<double, int>> MainComponent::generateRandomAmounts(
    double start, double end,
                                                      double range,
                                                      int length) {
  juce::Array<std::pair<double, int>> arr;
  maxAmount = DBL_MIN;
  minAmount = DBL_MAX;
  int noteDurationInSamples = std::ceil(srate / (playbackBpm / 60.0));
  for (int i = 0; i < length; i++) {
    double a = random.nextDouble() * range;
    double b = random.nextDouble() * 2 - 1;
    double c = random.nextDouble() * (range / 4);
    double d = random.nextDouble() * 2 - 1;
    double x = (end - start) * (i / length);

    double amount = generateRandomAmount(a, b, c, d, x);
    if (amount < minAmount) minAmount = amount;
    if (amount > maxAmount) maxAmount = amount;
    arr.add({amount, noteDurationInSamples});
  }
  return arr;
}

double MainComponent::generateRandomAmount(double a, double b, double c,
    double d, double x) {
  return a * cos(b * x) + c * sin(d * x) + a + c;
}

double MainComponent::mapAmount(double low1, double high1, double low2, double high2, double amount) {
  auto range1 = high1 - low1;
  auto range2 = high2 - low2;

  auto pointPosition = amount - low1;
  auto ratio = pointPosition / range1;

  return low2 + range2 * ratio;
}

void MainComponent::convertAmountsToNotes(
    juce::Array<std::pair<double, int>>& amounts) {
  for (auto& amountDurationPair : amounts) {
    double amount = amountDurationPair.first;
    // TODO: quantize note depending on scale
    // TODO: if using frequencies, change min/max midi pitch
    // to min/max frequency
    double note =
        mapAmount(minAmount, maxAmount, minMidiPitch, maxMidiPitch, amount);
    amountDurationPair.first = note;
  }
}

