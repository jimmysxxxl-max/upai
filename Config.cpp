#include "pch.h"
#include "Config.h"

namespace
{
    std::string Trim(std::string s)
    {
        const auto notSpace = [](unsigned char c) { return !std::isspace(c); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
        s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
        return s;
    }

    std::string Lower(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return s;
    }

    bool ParseBool(std::string value, bool fallback)
    {
        value = Lower(Trim(std::move(value)));
        if (value == "1" || value == "true" || value == "yes" || value == "on") {
            return true;
        }
        if (value == "0" || value == "false" || value == "no" || value == "off") {
            return false;
        }
        return fallback;
    }
}

namespace UPR
{
    Config Config::Load(const std::filesystem::path& a_path)
    {
        Config cfg;
        std::ifstream in(a_path);
        if (!in) {
            REX::WARN("Config not found at {}; using defaults", a_path.string());
            return cfg;
        }

        std::string section;
        std::string line;
        while (std::getline(in, line)) {
            if (const auto comment = line.find_first_of(";#"); comment != std::string::npos) {
                line.resize(comment);
            }
            line = Trim(std::move(line));
            if (line.empty()) {
                continue;
            }
            if (line.front() == '[' && line.back() == ']') {
                section = Lower(Trim(line.substr(1, line.size() - 2)));
                continue;
            }

            const auto eq = line.find('=');
            if (eq == std::string::npos) {
                continue;
            }

            auto key = Lower(Trim(line.substr(0, eq)));
            auto value = Trim(line.substr(eq + 1));

            if (section == "general") {
                if (key == "enableskinmeshes") cfg.enableSkinMeshes = ParseBool(value, cfg.enableSkinMeshes);
                else if (key == "enableskintextures") cfg.enableSkinTextures = ParseBool(value, cfg.enableSkinTextures);
                else if (key == "enablefirstperson") cfg.enableFirstPerson = ParseBool(value, cfg.enableFirstPerson);
                else if (key == "enablefacetextures") cfg.enableFaceTextures = ParseBool(value, cfg.enableFaceTextures);
                else if (key == "facebasenamefallback") cfg.faceBasenameFallback = ParseBool(value, cfg.faceBasenameFallback);
                else if (key == "requireloosetarget") cfg.requireLooseTarget = ParseBool(value, cfg.requireLooseTarget);
                else if (key == "reset3donapply") cfg.reset3DOnApply = ParseBool(value, cfg.reset3DOnApply);
                else if (key == "savehygiene") cfg.saveHygiene = ParseBool(value, cfg.saveHygiene);
                else if (key == "reapplyaftermenuclose") cfg.reapplyAfterMenuClose = ParseBool(value, cfg.reapplyAfterMenuClose);
                else if (key == "verboselog") cfg.verboseLog = ParseBool(value, cfg.verboseLog);
            } else if (section == "paths") {
                if (key == "meshfrom") cfg.meshFrom = value;
                else if (key == "meshto") cfg.meshTo = value;
                else if (key == "femaletexturefrom") cfg.femaleTextureFrom = value;
                else if (key == "femaletextureto") cfg.femaleTextureTo = value;
                else if (key == "maletexturefrom") cfg.maleTextureFrom = value;
                else if (key == "maletextureto") cfg.maleTextureTo = value;
            }
        }

        return cfg;
    }
}
