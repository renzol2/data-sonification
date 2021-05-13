#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent,
                      public juce::ComboBox::Listener,
                      public juce::Slider::Listener,
                      public juce::Button::Listener {
 public:
  //==============================================================================
  MainComponent();
  ~MainComponent() override;

  //==============================================================================
  void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
  void getNextAudioBlock(
      const juce::AudioSourceChannelInfo& bufferToFill) override;
  void releaseResources() override;

  //==============================================================================
  void paint(juce::Graphics& g) override;
  void resized() override;

  //==============================================================================
  void sliderValueChanged(Slider* slider) override;
  void comboBoxChanged(ComboBox* menu) override;
  void buttonClicked(Button* button) override;

  //==============================================================================
  bool isPlaying();
  void drawPlayButton(juce::DrawableButton& b, bool drawPlay);
  double inline phasor();
  void inline generateSine(const AudioSourceChannelInfo& bufferToFill,
                           int noteDuration);
  void inline generateSquare(const AudioSourceChannelInfo& bufferToFill,
                             int noteDuration);
  void inline generateTriangle(const AudioSourceChannelInfo& bufferToFill,
                               int noteDuration);
  void inline generateSaw(const AudioSourceChannelInfo& bufferToFill,
                          int noteDuration);
  /**
   * Returns true if playback has ended, or false otherwise
   */
  bool decrementNoteDurations();
  float inline getRandomSample();
  float inline getRandomSample(float amp);
  int convertFreqToMidi(double freq);
  double convertMidiToFreq(int midi);
  juce::Array<double> generateRandomAmounts(double start, double end,
                                            double range, int length);
  juce::Array<std::pair<double, int>> convertAmountsToNotes(
      const juce::Array<double>& amounts);
  double generateRandomAmount(double a, double b, double c, double d, double x);
  double mapAmount(double low1, double high1, double low2, double high2,
                   double amount);
  int quantizeNote(double amount);

 private:
  //==============================================================================

  enum OscillatorId { kNoOscilator, kSine, kSquare, kTriangle, kSaw };

  enum ScaleId { kNoScale, kChromatic, kDiatonic, kPentatonic, kWholeTone };

  const juce::Array<int> kDiatonicPitches{0, 2, 4, 5, 7, 9, 11};
  const juce::Array<int> kPentatonicPitches{0, 2, 4, 7, 9};
  const juce::Array<int> kWholeTonePitches{0, 2, 4, 6, 8, 10};
  const int kNumPitchClasses = 12;

  AudioSourcePlayer audioSourcePlayer;
  double phase = 0.0;
  double srate = 0.0;
  double phaseDelta = 0.0;
  double currentFreq = 0.0;
  const double TwoPi{double_Pi * 2.0};
  juce::Array<double> midiToFreqTable;
  juce::Array<double> amountsToPlay;
  juce::Array<std::pair<double, int>> notesToPlay;
  int currentAmountIndex = 0;
  double maxAmount = DBL_MIN;
  double minAmount = DBL_MAX;

  Rectangle<int> graphArea;

  Random random;

  DrawableButton playButton{"", juce::DrawableButton::ImageOnButtonBackground};
  Label playLabel{"playLabel", "Play data"};

  ComboBox dataMenu;
  Label dataLabel{"dataLabel", "Data: "};
  // TODO: add variable for currently selected data

  ComboBox oscillatorMenu;
  Label oscillatorLabel{"oscillatorLabel", "Oscillator: "};
  OscillatorId oscillatorId{kNoOscilator};

  ComboBox scaleMenu;
  Label scaleLabel{"scalelabel", "Scale: "};
  ScaleId scaleId{kNoScale};

  Slider levelSlider;
  Label levelLabel{"levelLabel", "Level: "};
  double level = 0.5;
  const double kMinLevel = 0.0;
  const double kMaxLevel = 1.0;

  Slider minPitchSlider;
  Label minPitchLabel{"minPitchLabel", "Min. pitch"};
  int minMidiPitch = 48;
  double minFreqPitch = 200.0;
  const double kMinMidiPitch = 0;
  const double kMinFreqPitch = 20.0;

  Slider maxPitchSlider;
  Label maxPitchLabel{"maxPitchSlider", "Max. pitch"};
  int maxMidiPitch = 72;
  double maxFreqPitch = 10000;
  const int kMaxMidiPitch = 127;
  const double kMaxFreqPitch = 18000;

  Slider playbackBpmSlider;
  Label playbackBpmLabel{"playbackBpmLabel", "BPM"};
  const int kMinBpm = 0;
  const int kMaxBpm = 999;
  int playbackBpm = 200;

  ToggleButton minMaxUnitButton{"Use MIDI pitch"};
  bool useMidi = false;

  // TODO: figure out how to make graph

  Font textFont{"Arial", 15.0f, Font::FontStyleFlags::plain};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
