#include "services/RoleConfigService.h"

#include <algorithm>
#include <cctype>

namespace wap {

namespace {

bool EqualsIgnoreCase(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

std::optional<RoleSuggestion> PickFrom(std::vector<RoleSuggestion> roles, std::mt19937& rng) {
    if (roles.empty()) {
        return std::nullopt;
    }
    std::uniform_int_distribution<size_t> dist(0, roles.size() - 1);
    return roles[dist(rng)];
}

}  // namespace

RoleConfigService::RoleConfigService(RoleConfig config) : config_(std::move(config)) {}

std::vector<RoleSuggestion> RoleConfigService::GetRolesByType(RoleType roleType) const {
    if (roleType == RoleType::FullRandom) {
        return config_.roles;
    }

    std::vector<RoleSuggestion> filtered;
    for (const RoleSuggestion& role : config_.roles) {
        switch (roleType) {
            case RoleType::DPS:
                if (role.roleType == RoleType::DPS && !role.providesQuickness &&
                    !role.providesAlacrity) {
                    filtered.push_back(role);
                }
                break;
            case RoleType::PowerDPS:
                if (role.roleType == RoleType::DPS && EqualsIgnoreCase(role.role, "Power") &&
                    !role.providesQuickness && !role.providesAlacrity) {
                    filtered.push_back(role);
                }
                break;
            case RoleType::ConditionDPS:
                if (role.roleType == RoleType::DPS && EqualsIgnoreCase(role.role, "Condition") &&
                    !role.providesQuickness && !role.providesAlacrity) {
                    filtered.push_back(role);
                }
                break;
            case RoleType::Healer:
                if (role.roleType == RoleType::Healer) {
                    filtered.push_back(role);
                }
                break;
            default:
                break;
        }
    }
    return filtered;
}

std::optional<RoleSuggestion> RoleConfigService::PickRandom(RoleType roleType,
                                                            std::mt19937& rng) const {
    return PickFrom(GetRolesByType(roleType), rng);
}

std::vector<RoleSuggestion> RoleConfigService::GetDPSWithBoon(bool providesQuickness,
                                                              bool providesAlacrity) const {
    std::vector<RoleSuggestion> filtered;
    for (const RoleSuggestion& role : config_.roles) {
        if (role.roleType == RoleType::DPS && role.providesQuickness == providesQuickness &&
            role.providesAlacrity == providesAlacrity) {
            filtered.push_back(role);
        }
    }
    return filtered;
}

std::vector<RoleSuggestion> RoleConfigService::GetHealerWithBoon(bool providesQuickness,
                                                                 bool providesAlacrity) const {
    std::vector<RoleSuggestion> filtered;
    for (const RoleSuggestion& role : config_.roles) {
        if (role.roleType == RoleType::Healer && role.providesQuickness == providesQuickness &&
            role.providesAlacrity == providesAlacrity) {
            filtered.push_back(role);
        }
    }
    return filtered;
}

const ProfessionInfo* RoleConfigService::GetProfessionByName(const std::string& name) const {
    for (const ProfessionInfo& profession : config_.professions) {
        if (EqualsIgnoreCase(profession.name, name)) {
            return &profession;
        }
    }
    return nullptr;
}

const EliteSpecInfo* RoleConfigService::GetEliteSpecByName(const std::string& name) const {
    for (const EliteSpecInfo& eliteSpec : config_.eliteSpecs) {
        if (EqualsIgnoreCase(eliteSpec.name, name)) {
            return &eliteSpec;
        }
    }
    return nullptr;
}

}  // namespace wap
