#pragma once

namespace UPR
{
    struct Config
    {
        bool enableSkinMeshes{ true };
        bool enableSkinTextures{ true };
        bool enableFirstPerson{ true };
        bool enableFaceTextures{ true };
        bool faceBasenameFallback{ true };
        bool requireLooseTarget{ true };
        bool reset3DOnApply{ true };
        bool saveHygiene{ true };
        bool reapplyAfterMenuClose{ true };
        bool verboseLog{ true };

        std::string meshFrom{ R"(Actors\Character\CharacterAssets\)" };
        std::string meshTo{ R"(Actors\Character\PlayerCharacterAssets\)" };
        std::string femaleTextureFrom{ R"(Actors\Character\BaseHumanFemale\)" };
        std::string femaleTextureTo{ R"(Actors\Character\PlayerHumanFemale\)" };
        std::string maleTextureFrom{ R"(Actors\Character\BaseHumanMale\)" };
        std::string maleTextureTo{ R"(Actors\Character\PlayerHumanMale\)" };

        static Config Load(const std::filesystem::path& a_path);
    };
}
