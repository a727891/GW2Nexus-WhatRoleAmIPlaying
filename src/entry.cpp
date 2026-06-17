#include "core/AppState.h"
#include "core/Branding.h"
#include "core/MumbleUtils.h"
#include "ui/DatAssetIconService.h"
#include "ui/MainWindow.h"
#include "ui/OptionsPanel.h"
#include "ui/QuickAccessService.h"

#include <imgui.h>
#include "mumble/Mumble.h"
#include "nexus/Nexus.h"
#include <windows.h>

namespace {

AddonAPI_t* g_api = nullptr;

constexpr const char* kToggleBind = "WAP_TOGGLE";

void AddonLoad(AddonAPI_t* api);
void AddonUnload();
void AddonRender();
void AddonOptions();

void RefreshDataLinks(wap::AppState& state) {
    if (!g_api) {
        return;
    }

    state.nexusLink = static_cast<NexusLinkData_t*>(g_api->DataLink_Get(DL_NEXUS_LINK));
    state.mumbleLink = static_cast<Mumble::Data*>(g_api->DataLink_Get(DL_MUMBLE_LINK));
}

void OnToggle(const char*, bool isRelease) {
    if (isRelease) {
        return;
    }
    wap::QuickAccessService::OnShortcutActivated(wap::AppState::Instance());
}

void AddonLoad(AddonAPI_t* api) {
    g_api = api;
    api->Log(LOGL_INFO, wap::kLogChannel, "AddonLoad starting.");

    ImGui::SetCurrentContext(static_cast<ImGuiContext*>(api->ImguiContext));
    ImGui::SetAllocatorFunctions(
        reinterpret_cast<void* (*)(size_t, void*)>(api->ImguiMalloc),
        reinterpret_cast<void (*)(void*, void*)>(api->ImguiFree));

    wap::AppState::Instance().PrepareLoad(api);

    api->GUI_Register(RT_Render, AddonRender);
    api->GUI_Register(RT_OptionsRender, AddonOptions);

    Keybind_t defaultBind{};
    api->InputBinds_RegisterWithStruct(kToggleBind, OnToggle, defaultBind);

    api->Log(LOGL_INFO, wap::kLogChannel, "Loaded.");
}

void AddonUnload() {
    if (!g_api) {
        return;
    }

    g_api->GUI_Deregister(AddonRender);
    g_api->GUI_Deregister(AddonOptions);
    g_api->InputBinds_Deregister(kToggleBind);

    wap::AppState::Instance().Shutdown();
    g_api->Log(LOGL_INFO, wap::kLogChannel, "Unloaded.");
    g_api = nullptr;
}

void AddonRender() {
    if (!g_api) {
        return;
    }

    auto& state = wap::AppState::Instance();
    RefreshDataLinks(state);

    if (!wap::IsInGame(state.mumbleLink, state.nexusLink)) {
        return;
    }

    state.ProcessInit();
    if (!state.IsReady()) {
        return;
    }

    wap::DatAssetIconService::ProcessDownloads();
    state.ProcessPendingConfigSync();
    state.ProcessBackgroundNotices();
    wap::QuickAccessService::SyncVisibility(g_api, state);

    wap::MainWindow::Render(state);
}

void AddonOptions() {
    if (!g_api) {
        return;
    }

    auto& state = wap::AppState::Instance();
    RefreshDataLinks(state);
    state.ProcessInit();
    wap::OptionsPanel::RenderNexusConfigEntry(state);
}

}  // namespace

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID) { return TRUE; }

extern "C" __declspec(dllexport) AddonDefinition_t* GetAddonDef() {
    static AddonDefinition_t def{};
    static bool initialized = false;
    if (!initialized) {
        def.Signature = -2024061501;
        def.APIVersion = NEXUS_API_VERSION;
        def.Name = wap::kDisplayName;
        def.Version = {1, 0, 0, 0};
        def.Author = wap::kAuthor;
        def.Description = wap::kDescription;
        def.Load = AddonLoad;
        def.Unload = AddonUnload;
        def.Flags = AF_None;
        def.Provider = UP_None;
        initialized = true;
    }
    return &def;
}
