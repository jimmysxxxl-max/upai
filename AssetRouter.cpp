#include "pch.h"
#include "AssetRouter.h"

namespace
{
    char LowerChar(char c)
    {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    bool IStartsWith(std::string_view value, std::string_view prefix)
    {
        if (value.size() < prefix.size()) {
            return false;
        }
        for (std::size_t i = 0; i < prefix.size(); ++i) {
            const char a = value[i] == '/' ? '\\' : value[i];
            const char b = prefix[i] == '/' ? '\\' : prefix[i];
            if (LowerChar(a) != LowerChar(b)) {
                return false;
            }
        }
        return true;
    }

    std::string NormalizeSlashes(std::string s)
    {
        std::replace(s.begin(), s.end(), '/', '\\');
        return s;
    }

    std::string FilenameOnly(std::string_view path)
    {
        auto normalized = NormalizeSlashes(std::string(path));
        const auto slash = normalized.find_last_of('\\');
        return slash == std::string::npos ? normalized : normalized.substr(slash + 1);
    }
}

namespace UPR
{
    AssetRouter::AssetRouter(Config a_config, std::filesystem::path a_gameRoot) :
        _config(std::move(a_config)),
        _gameRoot(std::move(a_gameRoot))
    {}

    std::optional<std::string> AssetRouter::ReplacePrefix(
        std::string_view a_path,
        std::string_view a_from,
        std::string_view a_to,
        AssetKind a_kind) const
    {
        if (a_path.empty() || !IStartsWith(a_path, a_from)) {
            return std::nullopt;
        }

        std::string result{ a_to };
        result.append(a_path.substr(a_from.size()));
        result = NormalizeSlashes(std::move(result));

        if (_config.requireLooseTarget && !TargetExists(a_kind, result)) {
            return std::nullopt;
        }
        return result;
    }

    std::string AssetRouter::TextureTargetPrefix(RE::SEX a_sex) const
    {
        return a_sex == RE::SEX::kFemale ? _config.femaleTextureTo : _config.maleTextureTo;
    }

    std::optional<std::string> AssetRouter::RedirectMesh(std::string_view a_path) const
    {
        if (const auto normal = ReplacePrefix(a_path, _config.meshFrom, _config.meshTo, AssetKind::Mesh)) {
            return normal;
        }

        // Hand/body replacers sometimes point their ARMA at a custom source folder rather
        // than CharacterAssets. Preserve the winning filename and look for that same file
        // in PlayerCharacterAssets instead of silently missing the redirect.
        if (!_config.skinMeshBasenameFallback || a_path.empty()) {
            return std::nullopt;
        }

        const auto filename = FilenameOnly(a_path);
        if (filename.empty()) {
            return std::nullopt;
        }

        auto target = NormalizeSlashes(_config.meshTo + filename);
        if (_config.requireLooseTarget && !TargetExists(AssetKind::Mesh, target)) {
            return std::nullopt;
        }
        return target;
    }

    std::optional<std::string> AssetRouter::RedirectTexture(std::string_view a_path, RE::SEX a_sex) const
    {
        const bool female = a_sex == RE::SEX::kFemale;
        const auto& from = female ? _config.femaleTextureFrom : _config.maleTextureFrom;
        const auto& to = female ? _config.femaleTextureTo : _config.maleTextureTo;

        if (const auto normal = ReplacePrefix(a_path, from, to, AssetKind::Texture)) {
            return normal;
        }

        if (!_config.skinTextureBasenameFallback || a_path.empty()) {
            return std::nullopt;
        }

        const auto filename = FilenameOnly(a_path);
        if (filename.empty()) {
            return std::nullopt;
        }

        auto target = NormalizeSlashes(TextureTargetPrefix(a_sex) + filename);
        if (_config.requireLooseTarget && !TargetExists(AssetKind::Texture, target)) {
            return std::nullopt;
        }
        return target;
    }

    std::optional<std::string> AssetRouter::RedirectFaceTexture(std::string_view a_path, RE::SEX a_sex) const
    {
        // Keep face fallback independent from the skin-texture fallback switch.
        const bool female = a_sex == RE::SEX::kFemale;
        const auto& from = female ? _config.femaleTextureFrom : _config.maleTextureFrom;
        const auto& to = female ? _config.femaleTextureTo : _config.maleTextureTo;
        if (const auto normal = ReplacePrefix(a_path, from, to, AssetKind::Texture)) {
            return normal;
        }

        if (!_config.faceBasenameFallback || a_path.empty()) {
            return std::nullopt;
        }

        const auto filename = FilenameOnly(a_path);
        if (filename.empty()) {
            return std::nullopt;
        }

        auto target = NormalizeSlashes(TextureTargetPrefix(a_sex) + filename);
        if (_config.requireLooseTarget && !TargetExists(AssetKind::Texture, target)) {
            return std::nullopt;
        }
        return target;
    }

    bool AssetRouter::TargetExists(AssetKind a_kind, std::string_view a_relativePath) const
    {
        const auto root = a_kind == AssetKind::Mesh ? "Meshes" : "Textures";
        std::filesystem::path rel{ std::string(a_relativePath) };
        return std::filesystem::exists(_gameRoot / "Data" / root / rel);
    }
}
