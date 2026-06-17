#include "data/RoleConfigJson.h"

#include <nlohmann/json.hpp>

namespace wap {

namespace {

std::optional<std::string> ReadOptionalString(const nlohmann::json& j, const char* key) {
    if (!j.contains(key) || j[key].is_null()) {
        return std::nullopt;
    }
    if (j[key].is_string()) {
        const auto value = j[key].get<std::string>();
        if (value.empty()) {
            return std::nullopt;
        }
        return value;
    }
    return std::nullopt;
}

RoleSuggestion RoleSuggestionFromJson(const nlohmann::json& j) {
    RoleSuggestion role;
    if (j.contains("profession")) {
        role.profession = j["profession"].get<std::string>();
    }
    if (j.contains("elite_spec")) {
        role.eliteSpec = j["elite_spec"].get<std::string>();
    }
    if (j.contains("role")) {
        role.role = j["role"].get<std::string>();
    }
    if (j.contains("role_type")) {
        role.roleType = RoleTypeFromString(j["role_type"].get<std::string>());
    }
    if (j.contains("provides_quickness")) {
        role.providesQuickness = j["provides_quickness"].get<bool>();
    }
    if (j.contains("provides_alacrity")) {
        role.providesAlacrity = j["provides_alacrity"].get<bool>();
    }
    if (j.contains("description")) {
        role.description = j["description"].get<std::string>();
    }
    if (j.contains("build_url")) {
        role.buildUrl = j["build_url"].get<std::string>();
    }
    return role;
}

ProfessionInfo ProfessionFromJson(const nlohmann::json& j) {
    ProfessionInfo info;
    if (j.contains("name")) {
        info.name = j["name"].get<std::string>();
    }
    if (j.contains("id")) {
        info.id = j["id"].get<int>();
    }
    if (j.contains("icon")) {
        info.icon = j["icon"].get<std::string>();
    }
    return info;
}

EliteSpecInfo EliteSpecFromJson(const nlohmann::json& j) {
    EliteSpecInfo info;
    if (j.contains("name")) {
        info.name = j["name"].get<std::string>();
    }
    if (j.contains("id")) {
        info.id = j["id"].get<int>();
    }
    if (j.contains("profession")) {
        info.profession = j["profession"].get<std::string>();
    }
    if (j.contains("icon")) {
        info.icon = j["icon"].get<std::string>();
    }
    if (j.contains("background")) {
        info.background = j["background"].get<std::string>();
    }
    return info;
}

}  // namespace

RoleConfig RoleConfigJson::FromJson(const nlohmann::json& j) {
    RoleConfig config;
    if (j.contains("version")) {
        config.version = j["version"].get<std::string>();
    }
    if (j.contains("last_updated")) {
        config.lastUpdated = j["last_updated"].get<std::string>();
    }
    if (j.contains("motd")) {
        config.motd = ReadOptionalString(j, "motd");
    }
    if (j.contains("motd_id")) {
        config.motdId = ReadOptionalString(j, "motd_id");
    }
    if (j.contains("roles") && j["roles"].is_array()) {
        for (const auto& item : j["roles"]) {
            config.roles.push_back(RoleSuggestionFromJson(item));
        }
    }
    if (j.contains("professions") && j["professions"].is_array()) {
        for (const auto& item : j["professions"]) {
            config.professions.push_back(ProfessionFromJson(item));
        }
    }
    if (j.contains("elite_specs") && j["elite_specs"].is_array()) {
        for (const auto& item : j["elite_specs"]) {
            config.eliteSpecs.push_back(EliteSpecFromJson(item));
        }
    }
    return config;
}

std::optional<RoleConfig> RoleConfigJson::Parse(const std::string& jsonText) {
    try {
        return FromJson(nlohmann::json::parse(jsonText));
    } catch (...) {
        return std::nullopt;
    }
}

}  // namespace wap
