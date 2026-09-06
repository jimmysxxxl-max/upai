#pragma once

#include "Config.h"

namespace UPR
{
    enum class AssetKind
    {
        Mesh,
        Texture
    };

    class AssetRouter
    {
    public:
        AssetRouter(Config a_config, std::filesystem::path a_gameRoot);

        [[nodiscard]] const Config& GetConfig() const noexcept { return _config; }

        [[nodiscard]] std::optional<std::string> RedirectMesh(std::string_view a_path) const;
        [[nodiscard]] std::optional<std::string> RedirectTexture(std::string_view a_path, RE::SEX a_sex) const;
        [[nodiscard]] std::optional<std::string> RedirectFaceTexture(std::string_view a_path, RE::SEX a_sex) const;
        [[nodiscard]] bool TargetExists(AssetKind a_kind, std::string_view a_relativePath) const;

    private:
        [[nodiscard]] std::optional<std::string> ReplacePrefix(
            std::string_view a_path,
            std::string_view a_from,
            std::string_view a_to,
            AssetKind a_kind) const;

        [[nodiscard]] std::string TextureTargetPrefix(RE::SEX a_sex) const;

        Config _config;
        std::filesystem::path _gameRoot;
    };
}
