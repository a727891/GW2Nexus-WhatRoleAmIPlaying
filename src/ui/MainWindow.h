#pragma once

#include "core/AppState.h"
#include "data/RoleModels.h"

namespace wap {

namespace MainWindow {

void Open();
void Close();
void Toggle();
bool IsOpen();

void Render(AppState& state);

void PickRandomRole(AppState& state, RoleType roleType);
void PickRandomDPSWithBoon(AppState& state, bool providesQuickness, bool providesAlacrity);
void PickRandomHealerWithBoon(AppState& state, bool providesQuickness, bool providesAlacrity);

}  // namespace MainWindow

}  // namespace wap
