#pragma once

#include "models/AudioAnalysis.h"
#include "models/Insight.h"
#include "models/Issue.h"

#include <optional>
#include <string>
#include <vector>

namespace sonora
{

enum class BackendStatus
{
    Offline,
    Connected,
    Uploading,
    Processing,
    Ready,
    Failed
};

class AppState
{
public:
    BackendStatus backendStatus() const { return backendStatus_; }
    void setBackendStatus(BackendStatus status) { backendStatus_ = status; }

    const std::optional<models::AudioAnalysis>& analysis() const { return analysis_; }
    const std::vector<models::Issue>& issues() const { return issues_; }
    const std::vector<models::Insight>& insights() const { return insights_; }
    const std::string& loadedFilename() const { return loadedFilename_; }

    // ApiClient and AudioLoader attach here later. UI never calls the network.

private:
    BackendStatus backendStatus_ { BackendStatus::Offline };
    std::optional<models::AudioAnalysis> analysis_;
    std::vector<models::Issue> issues_;
    std::vector<models::Insight> insights_;
    std::string loadedFilename_;
};

} // namespace sonora
