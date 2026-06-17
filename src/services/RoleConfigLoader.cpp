#include "services/RoleConfigLoader.h"

#include "core/Branding.h"
#include "data/RoleConfigJson.h"
#include "services/HttpClient.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace wap {

namespace {

constexpr const char* kCacheSubdir = "whatAmIPlaying";
constexpr const char* kRolesFilename = "roles.json";
constexpr const char* kMetaFilename = "roles_meta.json";

std::filesystem::path CacheDir(const std::string& addonDir) {
    return std::filesystem::path(addonDir) / kCacheSubdir;
}

std::filesystem::path RolesPath(const std::string& addonDir) {
    return CacheDir(addonDir) / kRolesFilename;
}

std::filesystem::path MetaPath(const std::string& addonDir) {
    return CacheDir(addonDir) / kMetaFilename;
}

bool WriteFile(const std::filesystem::path& path, const std::string& body) {
    std::error_code ec;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), ec);
    }
    std::ofstream out(path);
    if (!out.is_open()) {
        return false;
    }
    out << body;
    return true;
}

std::string ReadFile(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return {};
    }
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

struct RoleConfigMeta {
    std::string lastUpdated;

    static RoleConfigMeta Load(const std::string& addonDir) {
        RoleConfigMeta meta;
        try {
            const auto text = ReadFile(MetaPath(addonDir));
            if (text.empty()) {
                return meta;
            }
            const auto j = nlohmann::json::parse(text);
            if (j.contains("last_updated")) {
                meta.lastUpdated = j["last_updated"].get<std::string>();
            }
        } catch (...) {
        }
        return meta;
    }

    static void Save(const std::string& addonDir, const RoleConfigMeta& meta) {
        nlohmann::json j = {{"last_updated", meta.lastUpdated}};
        WriteFile(MetaPath(addonDir), j.dump(2));
    }
};

}  // namespace

void RoleConfigLoader::SetAddonDir(std::string addonDir) {
    addonDir_ = std::move(addonDir);
}

void RoleConfigLoader::LoadCached() {
    const auto text = ReadFile(RolesPath(addonDir_));
    if (text.empty()) {
        return;
    }

    const auto parsed = RoleConfigJson::Parse(text);
    if (!parsed) {
        return;
    }

    std::lock_guard lock(mutex_);
    config_ = *parsed;
}

bool RoleConfigLoader::SyncRemote() {
    const auto body = HttpGetUrl(kRolesConfigUrl);
    if (!body || body->empty()) {
        return false;
    }

    const auto parsed = RoleConfigJson::Parse(*body);
    if (!parsed) {
        return false;
    }

    const RoleConfigMeta meta = RoleConfigMeta::Load(addonDir_);
    if (!parsed->lastUpdated.empty() && parsed->lastUpdated == meta.lastUpdated) {
        return false;
    }

    if (!WriteFile(RolesPath(addonDir_), *body)) {
        return false;
    }

    RoleConfigMeta updated;
    updated.lastUpdated = parsed->lastUpdated;
    RoleConfigMeta::Save(addonDir_, updated);

    std::lock_guard lock(mutex_);
    config_ = *parsed;
    return true;
}

RoleConfig RoleConfigLoader::GetConfig() const {
    std::lock_guard lock(mutex_);
    return config_;
}

}  // namespace wap
