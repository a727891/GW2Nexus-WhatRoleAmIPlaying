#pragma once

#include "data/RoleModels.h"

#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>

namespace wap {

class RoleConfigJson {
public:
    static RoleConfig FromJson(const nlohmann::json& j);
    static std::optional<RoleConfig> Parse(const std::string& jsonText);
};

}  // namespace wap
