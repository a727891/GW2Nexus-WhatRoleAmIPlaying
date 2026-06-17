#pragma once

#include "data/RoleModels.h"
#include "services/RoleConfigService.h"

#include <optional>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

namespace wap {

class RoleSelectionService {
public:
    bool HasUnlockedEliteSpecs() const;
    std::unordered_set<std::string> GetUnlockedEliteSpecs() const;

    std::optional<RoleSuggestion> PickRandomRole(const RoleConfigService& configService,
                                                 RoleType roleType, std::mt19937& rng) const;

    std::optional<RoleSuggestion> PickRandomDPSWithBoon(const RoleConfigService& configService,
                                                        bool providesQuickness,
                                                        bool providesAlacrity,
                                                        std::mt19937& rng) const;

    std::optional<RoleSuggestion> PickRandomHealerWithBoon(const RoleConfigService& configService,
                                                           bool providesQuickness,
                                                           bool providesAlacrity,
                                                           std::mt19937& rng) const;

private:
    std::vector<RoleSuggestion> FilterByUnlockedSpecs(
        const std::vector<RoleSuggestion>& roles) const;
};

}  // namespace wap
