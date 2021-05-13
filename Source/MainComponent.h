#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent,
                      public juce::ComboBox::Listener,
                      public juce::Slider::Listener {
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
