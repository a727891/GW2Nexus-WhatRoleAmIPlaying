#pragma once

#include <optional>
#include <string>
#include <vector>

namespace wap {

enum class RoleType {
    FullRandom,
    DPS,
    PowerDPS,
    ConditionDPS,
    Healer,
};

struct RoleSuggestion {
    std::string profession;
    std::string eliteSpec;
    std::string role;
    RoleType roleType = RoleType::DPS;
    bool providesQuickness = false;
    bool providesAlacrity = false;
    std::string description;
    std::string buildUrl;
};

struct ProfessionInfo {
    std::string name;
    int id = 0;
    std::string icon;
};

struct EliteSpecInfo {
    std::string name;
    int id = 0;
    std::string profession;
    std::string icon;
    std::string background;
};

struct RoleConfig {
    std::string version;
    std::string lastUpdated;
    std::vector<RoleSuggestion> roles;
    std::vector<ProfessionInfo> professions;
    std::vector<EliteSpecInfo> eliteSpecs;
    std::optional<std::string> motd;
    std::optional<std::string> motdId;
};

RoleType RoleTypeFromString(const std::string& value);
const char* RoleTypeToString(RoleType type);

}  // namespace wap
