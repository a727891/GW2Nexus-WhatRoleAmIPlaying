#pragma once

#include <cstdint>

namespace wap {

// Nexus signature 0x60018006 (What Role Am I Playing, index 06)
inline constexpr int32_t kSignature = 0x60018006;
inline constexpr const char* kDisplayName = "What Role Am I Playing?";
inline constexpr const char* kLogChannel = "What Role Am I Playing?";
inline constexpr const char* kAuthor = "Soeed";
inline constexpr const char* kDescription =
    "Suggest a random raid build to play next based on a role you select.";
inline constexpr const char* kSourceRepoUrl =
    "https://github.com/a727891/BlishHud-WhatRoleAmIPlaying";
inline constexpr const char* kRolesConfigUrl =
    "https://bhm.blishhud.com/Soeed.WhatRoleAmIPlaying/roles.json";
inline constexpr const char* kPatchNotesUrl =
    "https://pkgs.blishhud.com/Soeed.WhatRoleAmIPlaying.html";

}  // namespace wap
