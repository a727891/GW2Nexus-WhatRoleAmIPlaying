#include "core/AppState.h"

#include "core/Branding.h"
#include "ui/DatAssetIconService.h"
#include "ui/MainWindow.h"
#include "ui/QuickAccessService.h"

#include <filesystem>
#include <thread>

namespace wap {

AppState& AppState::Instance() {
    static AppState state;
    return state;
}

AppState::AppState() = default;

std::string AppState::settingsPath() const {
    return (std::filesystem::path(addonDir) / "settings.json").string();
}

void AppState::RebuildRoleConfigService() {
    roleConfigService = std::make_unique<RoleConfigService>(roleConfigLoader.GetConfig());
}

void AppState::PrepareLoad(AddonAPI_t* apiPtr) {
    api = apiPtr;
    nexusLink = static_cast<NexusLinkData_t*>(api->DataLink_Get(DL_NEXUS_LINK));
    mumbleLink = static_cast<Mumble::Data*>(api->DataLink_Get(DL_MUMBLE_LINK));
}

void AppState::ProcessInit() {
    if (loadInitialized_ || !api) {
        return;
    }

    addonDir = api->Paths_GetAddonDirectory("WhatAmIPlaying");
    settings.Load(settingsPath());

    roleConfigLoader.SetAddonDir(addonDir);
    roleConfigLoader.LoadCached();
    RebuildRoleConfigService();

    DatAssetIconService::Initialize(api, addonDir);
    QuickAccessService::Register(api, *this);

    configSyncPending.store(true);
    loadInitialized_ = true;

    api->Log(LOGL_INFO, kLogChannel, "Initialized.");
}

void AppState::ProcessPendingConfigSync() {
    if (!loadInitialized_ || !configSyncPending.load() || configSyncInProgress.load()) {
        return;
    }

    configSyncInProgress.store(true);
    std::thread([this]() {
        const bool updated = roleConfigLoader.SyncRemote();
        if (updated) {
            RebuildRoleConfigService();
            configUpdatedNotice.store(true);
            if (api) {
                QuickAccessService::RefreshMotd(*this);
            }
        }
        configSyncInProgress.store(false);
        configSyncPending.store(false);
    }).detach();
}

void AppState::ProcessBackgroundNotices() {
    if (!configUpdatedNotice.exchange(false) || !api) {
        return;
    }

    const RoleConfig config = roleConfigLoader.GetConfig();
    std::string message = "Role library updated";
    if (!config.version.empty()) {
        message += " (v" + config.version + ")";
    }
    message += ".";
    api->Log(LOGL_INFO, kLogChannel, message.c_str());
}

void AppState::Shutdown() {
    configSyncPending.store(false);
    if (api) {
        MainWindow::Shutdown(api);
        QuickAccessService::Unregister(api);
    }
    DatAssetIconService::Shutdown();

    if (api && loadInitialized_) {
        settings.Save(settingsPath());
    }

    loadInitialized_ = false;
    roleConfigService.reset();
}

}  // namespace wap
