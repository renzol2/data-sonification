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
  void inline generateSine(const AudioSourceChannelInfo& bufferToFill);
  void inline generateSquare(const AudioSourceChannelInfo& bufferToFill);
  void inline generateTriangle(const AudioSourceChannelInfo& bufferToFill);
  void inline generateSaw(const AudioSourceChannelInfo& bufferToFill);
  float inline getRandomSample();
  float inline getRandomSample(float amp);
  int convertFreqToMidi(double freq);
  double convertMidiToFreq(int midi);
  juce::Array<double> generateRandomAmounts(double start, double end, double range, int length);
  double generateRandomAmount(double a, double b, double c, double d, double x);

 private:
  //==============================================================================

  enum OscillatorId { kNoOscilator, kSine, kSquare, kTriangle, kSaw };

  enum ScaleId {
    kNoScale,
    kFrequency,
    kChromatic,
    kDiatonic,
    kPentatonic,
    kWholeTone
  };

  AudioSourcePlayer audioSourcePlayer;
  double phase = 0.0;
  double srate = 0.0;
  double phaseDelta = 0.0;
  double currentFreq = 0.0;
  const double TwoPi{double_Pi * 2.0};
  juce::Array<double> midiToFreqTable;
  int time;
  int leftoverSamplesToPlay = 0;
  juce::Array<double> amountsToPlay;

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
