#include "ui/OptionsPanel.h"

#include "core/AppState.h"
#include "core/Branding.h"
#include "ui/MainWindow.h"

#include <imgui.h>

namespace wap {
namespace OptionsPanel {

void RenderNexusConfigEntry(AppState& state) {
    ImGui::TextColored(ImVec4(0.95f, 0.78f, 0.35f, 1.0f), "%s", kDisplayName);
    ImGui::Spacing();
    ImGui::TextWrapped("%s", kDescription);
    ImGui::Spacing();

    if (state.roleConfigService) {
        const auto& config = state.roleConfigService->Config();
        ImGui::Text("Role library: %zu builds", config.roles.size());
        if (!config.version.empty()) {
            ImGui::Text("Data version: %s", config.version.c_str());
        }
    } else {
        ImGui::TextDisabled("Role library not loaded yet.");
    }

    ImGui::Spacing();
    if (ImGui::Checkbox("Quick access icon enabled", &state.settings.cornerIconEnabled)) {
        state.settings.Save(state.settingsPath());
    }

    ImGui::Spacing();
    const ImVec2 buttonSize(ImGui::GetContentRegionAvail().x, 52.0f);
    if (ImGui::Button("Open What Am I Playing?", buttonSize)) {
        MainWindow::Open();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextUnformatted("Thank you to:");
    ImGui::BulletText("Freesnöw - continued static data and config file hosting");
}

}  // namespace OptionsPanel
}  // namespace wap
