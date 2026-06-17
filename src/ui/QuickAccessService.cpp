#include "ui/QuickAccessService.h"

#include "core/AppState.h"
#include "core/Branding.h"
#include "data/RoleModels.h"
#include "ui/DatAssetIconService.h"
#include "ui/MainWindow.h"

#include <imgui.h>
#include <string>

namespace wap {
namespace QuickAccessService {
namespace {

constexpr const char* kShortcutId = "WAP_QUICKACCESS";
constexpr const char* kToggleBind = "WAP_TOGGLE";
constexpr const char* kContextMenuId = "WAP_CTX_MENU";
constexpr int kCornerIconAssetId = 156720;

bool registered_ = false;
std::string cachedTooltip_;
std::string cachedIconTexId_;

std::string DatTextureId(int assetId) {
    return "WAP_DAT_" + std::to_string(assetId);
}

bool TextureReady(AddonAPI_t* api, int assetId) {
    if (!api || !api->Textures_Get) {
        return false;
    }
    DatAssetIconService::Request(assetId);
    const Texture_t* texture = api->Textures_Get(DatTextureId(assetId).c_str());
    return texture && texture->Resource;
}

std::string BuildTooltip(const AppState& state) {
    std::string tooltip = "What Am I Playing?";
    if (state.roleConfigService) {
        const RoleConfig& config = state.roleConfigService->Config();
        if (config.motd && !config.motd->empty()) {
            tooltip += "\n\n";
            tooltip += *config.motd;
        }
    }
    return tooltip;
}

void RenderContextMenuWrapper() {
    AppState& state = AppState::Instance();
    if (ImGui::Selectable("Full Random")) {
        MainWindow::PickRandomRole(state, RoleType::FullRandom);
    }
    ImGui::Separator();
    if (ImGui::Selectable("DPS")) {
        MainWindow::PickRandomRole(state, RoleType::DPS);
    }
    if (ImGui::Selectable("DPS - Quickness")) {
        MainWindow::PickRandomDPSWithBoon(state, true, false);
    }
    if (ImGui::Selectable("DPS - Alacrity")) {
        MainWindow::PickRandomDPSWithBoon(state, false, true);
    }
    if (ImGui::Selectable("Healer - Any")) {
        MainWindow::PickRandomRole(state, RoleType::Healer);
    }
    if (ImGui::Selectable("Healer - Quickness")) {
        MainWindow::PickRandomHealerWithBoon(state, true, false);
    }
    if (ImGui::Selectable("Healer - Alacrity")) {
        MainWindow::PickRandomHealerWithBoon(state, false, true);
    }
    ImGui::Separator();
    if (ImGui::Selectable("Open Window")) {
        MainWindow::Open();
    }
}

}  // namespace

void Register(AddonAPI_t* api, AppState& state) {
    if (!api || registered_) {
        return;
    }

    if (!TextureReady(api, kCornerIconAssetId)) {
        return;
    }

    cachedIconTexId_ = DatTextureId(kCornerIconAssetId);
    cachedTooltip_ = BuildTooltip(state);

    api->QuickAccess_Add(kShortcutId, cachedIconTexId_.c_str(), cachedIconTexId_.c_str(),
                         kToggleBind, cachedTooltip_.c_str());
    api->QuickAccess_AddContextMenu(kContextMenuId, kShortcutId, RenderContextMenuWrapper);
    registered_ = true;
    api->Log(LOGL_INFO, kLogChannel, "QuickAccess icon registered.");
}

void Unregister(AddonAPI_t* api) {
    if (!api || !registered_) {
        return;
    }

    api->QuickAccess_RemoveContextMenu(kContextMenuId);
    api->QuickAccess_Remove(kShortcutId);
    registered_ = false;
    cachedTooltip_.clear();
    cachedIconTexId_.clear();
}

void RefreshMotd(AppState& state) {
    if (!state.api || !registered_) {
        return;
    }

    const auto tooltip = BuildTooltip(state);
    if (tooltip == cachedTooltip_) {
        return;
    }

    Unregister(state.api);
    Register(state.api, state);
}

void SyncVisibility(AddonAPI_t* api, AppState& state) {
    if (!api) {
        return;
    }

    DatAssetIconService::Request(kCornerIconAssetId);

    if (state.settings.cornerIconEnabled) {
        if (!registered_) {
            Register(api, state);
        } else {
            RefreshMotd(state);
        }
    } else {
        Unregister(api);
    }
}

void OnShortcutActivated(AppState& state) { MainWindow::Toggle(); }

void OnCornerHover(AppState& state) {
    if (!state.roleConfigService) {
        return;
    }

    const RoleConfig& config = state.roleConfigService->Config();
    if (!config.motdId || config.motdId->empty()) {
        return;
    }

    state.settings.lastShownMotdId = *config.motdId;
    state.settings.Save(state.settingsPath());
}

}  // namespace QuickAccessService
}  // namespace wap
