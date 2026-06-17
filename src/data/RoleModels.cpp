#include "data/RoleModels.h"

#include <algorithm>
#include <cctype>

namespace wap {

namespace {

std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

}  // namespace

RoleType RoleTypeFromString(const std::string& value) {
    const std::string lower = ToLower(value);
    if (lower == "dps") {
        return RoleType::DPS;
    }
    if (lower == "healer") {
        return RoleType::Healer;
    }
    return RoleType::DPS;
}

const char* RoleTypeToString(RoleType type) {
    switch (type) {
        case RoleType::Healer:
            return "Healer";
        case RoleType::FullRandom:
        case RoleType::PowerDPS:
        case RoleType::ConditionDPS:
        case RoleType::DPS:
        default:
            return "DPS";
    }
}

}  // namespace wap
