#include "core/SettingsStore.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace wap {

void SettingsStore::Load(const std::string& filePath) {
    std::ifstream in(filePath);
    if (!in.is_open()) {
        return;
    }

    nlohmann::json j;
    in >> j;

    if (j.contains("LastShownMotdId")) {
        lastShownMotdId = j["LastShownMotdId"].get<std::string>();
    }
    if (j.contains("cornerIconEnabled")) {
        cornerIconEnabled = j["cornerIconEnabled"].get<bool>();
    }
}

void SettingsStore::Save(const std::string& filePath) const {
    nlohmann::json j = {
        {"LastShownMotdId", lastShownMotdId},
        {"cornerIconEnabled", cornerIconEnabled},
    };

    std::filesystem::path path(filePath);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream out(filePath);
    out << j.dump(2);
}

}  // namespace wap
