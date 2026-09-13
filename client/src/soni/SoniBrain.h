#pragma once

#include "soni/SoniTypes.h"

namespace sonora::soni
{

juce::String greet(const Context& ctx);
juce::String reply(const Context& ctx, const juce::String& user);
juce::String afterListen(const Context& ctx);
juce::String afterFail(const juce::String& reason);

} // namespace sonora::soni
