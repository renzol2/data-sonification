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
      {"Frequency", "Chromatic", "Diatonic", "Pentatonic", "Whole Tone"}, kFrequency);

  // Initialize level slider
  levelSlider.setRange(kMinLevel, kMaxLevel);
  levelSlider.setSliderStyle(Slider::LinearHorizontal);
  levelSlider.setTextBoxStyle(Slider::TextBoxLeft, false, 90, 22);

  // Initialize minimum pitch slider
  minPitchSlider.setRange(kMinMidiPitch, kMaxMidiPitch);
  minPitchSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);

  // Initialize maximum pitch slider
  maxPitchSlider.setRange(kMinMidiPitch, kMaxMidiPitch);
  maxPitchSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);

  // Initialize BPM slider
  playbackBpmSlider.setRange(kMinBpm, kMaxBpm);
  playbackBpmSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);

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
}

void MainComponent::getNextAudioBlock(
    const juce::AudioSourceChannelInfo& bufferToFill) {
  // Your audio-processing code goes here!

  // For more details, see the help for AudioProcessor::getNextAudioBlock()

  // Right now we are not producing any data, in which case we need to clear the
  // buffer (to prevent the output of random noise)
  bufferToFill.clearActiveBufferRegion();
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

  componentBounds.removeFromTop(PADDING);  // padding between first and second row
  
  auto secondRow = componentBounds.removeFromTop(COL_HEIGHT);
  dataLabel.setBounds(secondRow.removeFromLeft(LABEL_WIDTH));
  dataMenu.setBounds(secondRow.removeFromLeft(MENU_WIDTH));

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

void MainComponent::sliderValueChanged(Slider* slider) {}

void MainComponent::comboBoxChanged(ComboBox* menu) {}