#pragma once

#include "data/RoleModels.h"

#include <mutex>
#include <string>

namespace wap {

class RoleConfigLoader {
public:
    void SetAddonDir(std::string addonDir);

    void LoadCached();
    bool SyncRemote();

    RoleConfig GetConfig() const;

private:
    std::string addonDir_;
    mutable std::mutex mutex_;
    RoleConfig config_;
};

}  // namespace wap
