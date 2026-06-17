#pragma once

#include "data/RoleModels.h"

#include <optional>
#include <random>
#include <vector>

namespace wap {

class RoleConfigService {
public:
    explicit RoleConfigService(RoleConfig config);

    const RoleConfig& Config() const { return config_; }

    std::optional<RoleSuggestion> PickRandom(RoleType roleType, std::mt19937& rng) const;
    std::vector<RoleSuggestion> GetRolesByType(RoleType roleType) const;
    std::vector<RoleSuggestion> GetDPSWithBoon(bool providesQuickness, bool providesAlacrity) const;
    std::vector<RoleSuggestion> GetHealerWithBoon(bool providesQuickness, bool providesAlacrity) const;

    const ProfessionInfo* GetProfessionByName(const std::string& name) const;
    const EliteSpecInfo* GetEliteSpecByName(const std::string& name) const;

private:
    RoleConfig config_;
};

}  // namespace wap
