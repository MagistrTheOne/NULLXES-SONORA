#pragma once

#include <juce_core/juce_core.h>

#include <atomic>

namespace sonora::soni
{

class Voice
{
public:
    static Voice& get();

    void speak(const juce::String& text);
    void silence();
    bool speaking() const { return speaking_.load(); }

private:
    Voice() = default;
    std::atomic<bool> speaking_ { false };
    std::atomic<int> generation_ { 0 };
};

} // namespace sonora::soni
