#include "ui/MainWindow.h"

#include "core/AppState.h"
#include "core/Branding.h"
#include "ui/DatAssetIconService.h"

#include "nexus/Nexus.h"

#include <imgui.h>
#include <shellapi.h>

#include <cstdlib>
#include <string>

namespace wap {
namespace MainWindow {
namespace {

constexpr int kDefaultIconAssetId = 156678;
constexpr int kQuicknessAssetId = 1012835;
constexpr int kAlacrityAssetId = 1938787;
constexpr const char* kWindowId = "What Role Am I Playing?###WAP_MAIN_WINDOW";

bool g_open = false;
bool g_escapeRegistered = false;
bool g_hasSelection = false;
RoleSuggestion g_selectedRole;
int g_professionIconAssetId = kDefaultIconAssetId;
int g_eliteSpecIconAssetId = kDefaultIconAssetId;
int g_boonIconAssetId = 0;

void RegisterCloseOnEscape(AddonAPI_t* api) {
    if (!api || !api->GUI_RegisterCloseOnEscape || g_escapeRegistered) {
        return;
    }
    api->GUI_RegisterCloseOnEscape(kWindowId, &g_open);
    g_escapeRegistered = true;
}

void DeregisterCloseOnEscape(AddonAPI_t* api) {
    if (!api || !api->GUI_DeregisterCloseOnEscape || !g_escapeRegistered) {
        return;
    }
    api->GUI_DeregisterCloseOnEscape(kWindowId);
    g_escapeRegistered = false;
}

void SyncEscapeRegistration(AddonAPI_t* api) {
    if (g_open) {
        RegisterCloseOnEscape(api);
    } else {
        DeregisterCloseOnEscape(api);
    }
}

void HandleEscapeKey() {
    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        return;
    }

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
        g_open = false;
    }
}

void ShowAlert(AppState& state, const char* message) {
    if (state.api && state.api->GUI_SendAlert) {
        state.api->GUI_SendAlert(message);
    }
}

int ParseAssetId(const std::string& value) {
    if (value.empty()) {
        return 0;
    }
    char* end = nullptr;
    const long parsed = std::strtol(value.c_str(), &end, 10);
    if (end == value.c_str() || parsed <= 0) {
        return 0;
    }
    return static_cast<int>(parsed);
}

void OpenBuildUrl(const std::string& url) {
    if (url.empty()) {
        return;
    }
    ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void DisplayRole(AppState& state, const RoleSuggestion& role) {
    if (!state.roleConfigService) {
        return;
    }

    g_selectedRole = role;
    g_hasSelection = true;

    g_professionIconAssetId = kDefaultIconAssetId;
    g_eliteSpecIconAssetId = kDefaultIconAssetId;
    g_boonIconAssetId = 0;

    if (const ProfessionInfo* profession =
            state.roleConfigService->GetProfessionByName(role.profession)) {
        if (const int assetId = ParseAssetId(profession->icon)) {
            g_professionIconAssetId = assetId;
        }
    }
    if (const EliteSpecInfo* eliteSpec =
            state.roleConfigService->GetEliteSpecByName(role.eliteSpec)) {
        if (const int assetId = ParseAssetId(eliteSpec->icon)) {
            g_eliteSpecIconAssetId = assetId;
        }
    }

    if (role.providesQuickness) {
        g_boonIconAssetId = kQuicknessAssetId;
    } else if (role.providesAlacrity) {
        g_boonIconAssetId = kAlacrityAssetId;
    }

    DatAssetIconService::Request(g_professionIconAssetId);
    DatAssetIconService::Request(g_eliteSpecIconAssetId);
    if (g_boonIconAssetId > 0) {
        DatAssetIconService::Request(g_boonIconAssetId);
    }

    Open();
}

void DrawIcon(int assetId, float size) {
    if (const ImTextureID texture = DatAssetIconService::Request(assetId)) {
        ImGui::Image(texture, ImVec2(size, size));
    } else {
        ImGui::Dummy(ImVec2(size, size));
    }
}

void RenderRolePanel() {
    if (!g_hasSelection) {
        ImGui::TextDisabled("Click a button above to get a role suggestion.");
        return;
    }

    DrawIcon(g_professionIconAssetId, 32.0f);
    ImGui::SameLine();
    DrawIcon(g_eliteSpecIconAssetId, 32.0f);
    if (g_boonIconAssetId > 0) {
        ImGui::SameLine();
        DrawIcon(g_boonIconAssetId, 32.0f);
    }

    ImGui::Spacing();
    ImGui::Text("%s - %s (%s)", g_selectedRole.profession.c_str(), g_selectedRole.eliteSpec.c_str(),
                g_selectedRole.role.c_str());
    ImGui::TextWrapped("%s", g_selectedRole.description.c_str());

    if (!g_selectedRole.buildUrl.empty()) {
        ImGui::Spacing();
        if (ImGui::Button("View Build")) {
            OpenBuildUrl(g_selectedRole.buildUrl);
        }
    }
}

bool RoleGridButton(const char* label, const ImVec2& size) {
    return ImGui::Button(label, size);
}

void HandleMissingRoles(AppState& state, const char* context) {
    std::string message = "No available roles for ";
    message += context;
    message += ".";
    ShowAlert(state, message.c_str());
}

}  // namespace

void Open() {
    g_open = true;
    RegisterCloseOnEscape(AppState::Instance().api);
}

void Close() {
    g_open = false;
    DeregisterCloseOnEscape(AppState::Instance().api);
}

void Toggle() {
    if (g_open) {
        Close();
        return;
    }
    Open();
}

bool IsOpen() { return g_open; }

void Shutdown(AddonAPI_t* api) {
    DeregisterCloseOnEscape(api);
    g_open = false;
}

void PickRandomRole(AppState& state, RoleType roleType) {
    if (!state.roleConfigService) {
        ShowAlert(state, "Role library is not loaded yet.");
        return;
    }
    if (!state.roleSelection.HasUnlockedEliteSpecs()) {
        ShowAlert(state,
                  "No unlocked elite specs found. Please unlock some elite specializations "
                  "first.");
        return;
    }

    const auto suggestion =
        state.roleSelection.PickRandomRole(*state.roleConfigService, roleType, state.rng);
    if (!suggestion) {
        HandleMissingRoles(state, "your selection");
        return;
    }

    DisplayRole(state, *suggestion);
    ShowAlert(state, ("Try: " + suggestion->description).c_str());
}

void PickRandomDPSWithBoon(AppState& state, bool providesQuickness, bool providesAlacrity) {
    if (!state.roleConfigService) {
        ShowAlert(state, "Role library is not loaded yet.");
        return;
    }
    if (!state.roleSelection.HasUnlockedEliteSpecs()) {
        ShowAlert(state,
                  "No unlocked elite specs found. Please unlock some elite specializations "
                  "first.");
        return;
    }

    const auto suggestion = state.roleSelection.PickRandomDPSWithBoon(
        *state.roleConfigService, providesQuickness, providesAlacrity, state.rng);
    if (!suggestion) {
        HandleMissingRoles(state, "that boon DPS filter");
        return;
    }

    DisplayRole(state, *suggestion);
    ShowAlert(state,
              ("Try: " + suggestion->profession + " - " + suggestion->eliteSpec + " (" +
               suggestion->role + ")")
                  .c_str());
}

void PickRandomHealerWithBoon(AppState& state, bool providesQuickness, bool providesAlacrity) {
    if (!state.roleConfigService) {
        ShowAlert(state, "Role library is not loaded yet.");
        return;
    }
    if (!state.roleSelection.HasUnlockedEliteSpecs()) {
        ShowAlert(state,
                  "No unlocked elite specs found. Please unlock some elite specializations "
                  "first.");
        return;
    }

    const auto suggestion = state.roleSelection.PickRandomHealerWithBoon(
        *state.roleConfigService, providesQuickness, providesAlacrity, state.rng);
    if (!suggestion) {
        HandleMissingRoles(state, "that healer filter");
        return;
    }

    DisplayRole(state, *suggestion);
    ShowAlert(state,
              ("Try: " + suggestion->profession + " - " + suggestion->eliteSpec + " (" +
               suggestion->role + ")")
                  .c_str());
}

void Render(AppState& state) {
    if (!g_open) {
        return;
    }

    RegisterCloseOnEscape(state.api);

    ImGui::SetNextWindowSize(ImVec2(425.0f, 425.0f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(kWindowId, &g_open, ImGuiWindowFlags_None)) {
        ImGui::End();
        SyncEscapeRegistration(state.api);
        return;
    }

    if (!state.roleConfigService || state.roleConfigService->Config().roles.empty()) {
        ImGui::TextWrapped("Loading role library...");
        if (state.configSyncInProgress.load()) {
            ImGui::TextDisabled("Syncing from static host...");
        }
        HandleEscapeKey();
        ImGui::End();
        SyncEscapeRegistration(state.api);
        return;
    }

    const float buttonWidth = 120.0f;
    const float buttonHeight = 32.0f;
    const ImVec2 buttonSize(buttonWidth, buttonHeight);

    const float randomWidth = 400.0f;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - randomWidth) * 0.5f);
    if (ImGui::Button("Random", ImVec2(randomWidth, 40.0f))) {
        PickRandomRole(state, RoleType::FullRandom);
    }

    ImGui::Spacing();

    if (RoleGridButton("DPS", buttonSize)) {
        PickRandomRole(state, RoleType::DPS);
    }
    ImGui::SameLine(0.0f, 20.0f);
    if (RoleGridButton("BoonDPS", buttonSize)) {
        PickRandomDPSWithBoon(state, true, false);
    }
    ImGui::SameLine(0.0f, 20.0f);
    if (RoleGridButton("Healer", buttonSize)) {
        PickRandomRole(state, RoleType::Healer);
    }

    if (RoleGridButton("Power", buttonSize)) {
        PickRandomRole(state, RoleType::PowerDPS);
    }
    ImGui::SameLine(0.0f, 20.0f);
    if (RoleGridButton("Quickness", buttonSize)) {
        PickRandomDPSWithBoon(state, true, false);
    }
    ImGui::SameLine(0.0f, 20.0f);
    if (RoleGridButton("QuickHeal", buttonSize)) {
        PickRandomHealerWithBoon(state, true, false);
    }

    if (RoleGridButton("Condi", buttonSize)) {
        PickRandomRole(state, RoleType::ConditionDPS);
    }
    ImGui::SameLine(0.0f, 20.0f);
    if (RoleGridButton("Alacrity", buttonSize)) {
        PickRandomDPSWithBoon(state, false, true);
    }
    ImGui::SameLine(0.0f, 20.0f);
    if (RoleGridButton("AlacHeal", buttonSize)) {
        PickRandomHealerWithBoon(state, false, true);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    RenderRolePanel();

    HandleEscapeKey();
    ImGui::End();
    SyncEscapeRegistration(state.api);
}

}  // namespace MainWindow
}  // namespace wap
