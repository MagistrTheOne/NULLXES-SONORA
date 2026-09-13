#pragma once

#include <juce_core/juce_core.h>

namespace sonora::soni
{

struct Message
{
    bool fromSoni = true;
    juce::String text;
};

struct Context
{
    int hour = 12;
    int minute = 0;
    bool plugin = false;
    bool hasTrack = false;
    bool analyzing = false;
    juce::String filename;
    juce::String bpm;
    juce::String key;
    juce::String style;
    juce::String energy;
    int health = 0;
    bool muddy = false;
    bool vocalFight = false;
    bool clip = false;
    bool hasDrop = false;
};

} // namespace sonora::soni
