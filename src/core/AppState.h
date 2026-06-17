#pragma once

#include "core/SettingsStore.h"
#include "services/RoleConfigLoader.h"
#include "services/RoleConfigService.h"
#include "services/RoleSelectionService.h"

#include "mumble/Mumble.h"
#include "nexus/Nexus.h"

#include <atomic>
#include <memory>
#include <random>
#include <string>

namespace wap {

class AppState {
public:
    static AppState& Instance();

    AddonAPI_t* api = nullptr;
    NexusLinkData_t* nexusLink = nullptr;
    Mumble::Data* mumbleLink = nullptr;
    std::string addonDir;

    SettingsStore settings;
    RoleConfigLoader roleConfigLoader;
    RoleSelectionService roleSelection;
    std::unique_ptr<RoleConfigService> roleConfigService;

    std::atomic<bool> configSyncPending{false};
    std::atomic<bool> configSyncInProgress{false};
    std::atomic<bool> configUpdatedNotice{false};

    std::mt19937 rng{std::random_device{}()};

    void PrepareLoad(AddonAPI_t* apiPtr);
    void ProcessInit();
    void ProcessPendingConfigSync();
    void ProcessBackgroundNotices();
    bool IsReady() const { return loadInitialized_; }
    void Shutdown();

    std::string settingsPath() const;

private:
    AppState();
    void RebuildRoleConfigService();

    bool loadInitialized_ = false;
};

}  // namespace wap
