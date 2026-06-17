#include "core/MumbleUtils.h"

namespace wap {

bool IsInGame(const Mumble::Data* mumble, const NexusLinkData_t* nexus) {
    return mumble != nullptr && nexus != nullptr && nexus->IsGameplay;
}

}  // namespace wap
