#pragma once

#include "mumble/Mumble.h"
#include "nexus/Nexus.h"

namespace wap {

bool IsInGame(const Mumble::Data* mumble, const NexusLinkData_t* nexus);

}  // namespace wap
