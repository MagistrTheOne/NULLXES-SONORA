#pragma once

#include "models/Issue.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace sonora
{

class IssueRow : public juce::Component
{
public:
    IssueRow();

    void setIssue(const models::Issue& issue);
    void setEmpty(const juce::String& message);

    void paint(juce::Graphics& g) override;

private:
    juce::String title_;
    juce::String detail_;
    float severity_ = 0.0f;
    bool empty_ = true;
};

} // namespace sonora
