#pragma once

#include <string>

namespace wap {

struct SettingsStore {
    bool cornerIconEnabled = true;
    std::string lastShownMotdId;

    void Load(const std::string& filePath);
    void Save(const std::string& filePath) const;
};

}  // namespace wap
