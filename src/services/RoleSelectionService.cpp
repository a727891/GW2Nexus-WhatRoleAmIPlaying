#include "services/RoleSelectionService.h"

namespace wap {

namespace {

std::optional<RoleSuggestion> PickFrom(std::vector<RoleSuggestion> roles, std::mt19937& rng) {
    if (roles.empty()) {
        return std::nullopt;
    }
    std::uniform_int_distribution<size_t> dist(0, roles.size() - 1);
    return roles[dist(rng)];
}

}  // namespace

bool RoleSelectionService::HasUnlockedEliteSpecs() const {
    return !GetUnlockedEliteSpecs().empty();
}

std::unordered_set<std::string> RoleSelectionService::GetUnlockedEliteSpecs() const {
    // Blish parity: all elite specs treated as unlocked until GW2 API unlock checks land.
    return {
        "Dragonhunter",  "Firebrand",   "Willbender",  "Berserker",    "Spellbreaker",
        "Bladesworn",    "Scrapper",    "Holosmith",   "Mechanist",    "Druid",
        "Soulbeast",     "Untamed",     "Daredevil",   "Deadeye",      "Specter",
        "Tempest",       "Weaver",      "Catalyst",    "Chronomancer", "Mirage",
        "Virtuoso",      "Reaper",      "Scourge",     "Harbinger",    "Herald",
        "Renegade",      "Vindicator",
    };
}

std::vector<RoleSuggestion> RoleSelectionService::FilterByUnlockedSpecs(
    const std::vector<RoleSuggestion>& roles) const {
    const auto unlocked = GetUnlockedEliteSpecs();
    std::vector<RoleSuggestion> filtered;
    filtered.reserve(roles.size());
    for (const RoleSuggestion& role : roles) {
        if (unlocked.contains(role.eliteSpec)) {
            filtered.push_back(role);
        }
    }
    return filtered;
}

std::optional<RoleSuggestion> RoleSelectionService::PickRandomRole(
    const RoleConfigService& configService, RoleType roleType, std::mt19937& rng) const {
    if (!HasUnlockedEliteSpecs()) {
        return std::nullopt;
    }
    return PickFrom(FilterByUnlockedSpecs(configService.GetRolesByType(roleType)), rng);
}

std::optional<RoleSuggestion> RoleSelectionService::PickRandomDPSWithBoon(
    const RoleConfigService& configService, bool providesQuickness, bool providesAlacrity,
    std::mt19937& rng) const {
    if (!HasUnlockedEliteSpecs()) {
        return std::nullopt;
    }
    return PickFrom(configService.GetDPSWithBoon(providesQuickness, providesAlacrity), rng);
}

std::optional<RoleSuggestion> RoleSelectionService::PickRandomHealerWithBoon(
    const RoleConfigService& configService, bool providesQuickness, bool providesAlacrity,
    std::mt19937& rng) const {
    if (!HasUnlockedEliteSpecs()) {
        return std::nullopt;
    }
    return PickFrom(configService.GetHealerWithBoon(providesQuickness, providesAlacrity), rng);
}

}  // namespace wap
