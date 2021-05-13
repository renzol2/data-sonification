#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent {
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

 private:
  //==============================================================================
  // Your private member variables go here...

  DrawableButton playButton{"", juce::DrawableButton::ImageOnButtonBackground};

  ComboBox dataMenu;
  Label dataLabel{"dataLabel", "Data: "};

  ComboBox oscillatorMenu;
  Label oscillatorLabel{"oscillatorLabel", "Oscillator: "};

  ComboBox scaleMenu;
  Label scaleLabel{"scalelabel", "Scale: "};

  Slider levelSlider;
  Label levelLabel{"levelLabel", "Level: "};

  Slider minPitchSlider;
  Label minPitchLabel{"minPitchLabel", "Min. pitch"};

  Slider maxPitchSlider;
  Label maxPitchLabel{"maxPitchSlider", "Max. pitch"};

  Slider playbackBpmSlider;
  Label playbackBpmLabel{"playbackBpmLabel", "Playback BPM"};

  ToggleButton minMaxUnitButton{"Use MIDI pitch"};

  // TODO: figure out how to make graph

  Font textFont{"Arial", 15.0f, Font::FontStyleFlags::plain};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
