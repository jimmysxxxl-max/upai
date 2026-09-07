#include "pch.h"
#include "RuntimeForms.h"

namespace UPR
{
    RuntimeForms::RuntimeForms(AssetRouter a_router) :
        _router(std::move(a_router))
    {}

    RE::TESNPC* RuntimeForms::GetPlayerNPC() const
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        return player ? player->GetNPC() : nullptr;
    }

    RE::TESObjectARMO* RuntimeForms::ResolveSkinSource() const
    {
        if (!_npc) {
            return nullptr;
        }
        if (_npc->formSkin) {
            return _npc->formSkin;
        }
        if (auto* race = _npc->GetFormRace()) {
            return race->formSkin;
        }
        return nullptr;
    }

    RE::BGSTextureSet* RuntimeForms::GetCurrentFaceHeadPartTexture() const
    {
        if (!_npc) {
            return nullptr;
        }

        for (auto* part : _npc->GetHeadParts(true)) {
            if (part && part->type == RE::BGSHeadPart::HeadPartType::kFace) {
                return part->textureSet;
            }
        }
        return nullptr;
    }

    RE::BGSTextureSet* RuntimeForms::ResolveFaceTextureSource(RE::SEX a_sex) const
    {
        if (!_npc) {
            return nullptr;
        }

        // An explicit NPC/runtime face TXST is the strongest upstream winner.
        if (_npc->headRelatedData && _npc->headRelatedData->faceDetails) {
            return _npc->headRelatedData->faceDetails;
        }

        // When there is no explicit NPC face TXST, inherit the current winning Face
        // HeadPart's texture set. This is important for custom/high-poly heads that
        // supply both a replacement NIF and their own compatible base TXST. We never
        // modify or replace the HeadPart itself.
        if (auto* headPartTexture = GetCurrentFaceHeadPartTexture()) {
            return headPartTexture;
        }

        const auto sex = static_cast<std::size_t>(a_sex == RE::SEX::kFemale ? 1 : 0);
        if (auto* race = _npc->GetFormRace(); race && race->faceRelatedData[sex]) {
            // Mirror the game's/LooksMenu-style race fallback as closely as the public
            // structures allow: prefer the race's Face HeadPart TXST, then its default
            // face-details TXST as the final fallback.
            if (auto* headParts = race->faceRelatedData[sex]->headParts) {
                for (auto* part : *headParts) {
                    if (part && part->type == RE::BGSHeadPart::HeadPartType::kFace && part->textureSet) {
                        return part->textureSet;
                    }
                }
            }
            return race->faceRelatedData[sex]->defaultFaceDetailsTextureSet;
        }
        return nullptr;
    }

    bool RuntimeForms::CanRedirectTextureSet(RE::BGSTextureSet* a_set, RE::SEX a_sex, bool a_face) const
    {
        if (!a_set) {
            return false;
        }

        for (const auto& texture : a_set->textures) {
            const char* source = texture.textureName.c_str();
            if (!source || !*source) {
                continue;
            }
            const auto redirected = a_face ?
                _router.RedirectFaceTexture(source, a_sex) :
                _router.RedirectTexture(source, a_sex);
            if (redirected) {
                return true;
            }
        }
        return false;
    }

    bool RuntimeForms::RedirectTextureSetInPlace(
        RE::BGSTextureSet* a_set,
        RE::SEX a_sex,
        bool a_face,
        std::size_t& a_redirectCount)
    {
        if (!a_set) {
            return false;
        }

        bool changed = false;
        for (std::size_t i = 0; i < std::size(a_set->textures); ++i) {
            const char* source = a_set->textures[i].textureName.c_str();
            if (!source || !*source) {
                continue;
            }

            const auto redirected = a_face ?
                _router.RedirectFaceTexture(source, a_sex) :
                _router.RedirectTexture(source, a_sex);

            if (redirected) {
                const std::string sourceCopy{ source };
                a_set->textures[i].textureName = redirected->c_str();
                a_set->textureFileIDs[i].GenerateFromPath(redirected->c_str());
                ++a_redirectCount;
                changed = true;
                if (_router.GetConfig().verboseLog) {
                    REX::INFO("{} texture: {} -> {}", a_face ? "Face" : "Skin", sourceCopy, *redirected);
                }
            }
        }
        return changed;
    }

    RE::BGSTextureSet* RuntimeForms::CloneSkinTextureSet(
        RE::BGSTextureSet* a_source,
        RE::SEX a_sex,
        std::size_t& a_redirectCount)
    {
        if (!a_source) {
            return nullptr;
        }
        if (const auto it = _skinTextureCopies.find(a_source); it != _skinTextureCopies.end()) {
            return it->second;
        }

        if (!CanRedirectTextureSet(a_source, a_sex, false)) {
            return a_source;
        }

        auto* copy = DuplicateTemporary(a_source);
        if (!copy) {
            REX::ERROR("Failed to duplicate skin TXST {:08X}", a_source->GetFormID());
            return a_source;
        }

        const auto before = a_redirectCount;
        if (!RedirectTextureSetInPlace(copy, a_sex, false, a_redirectCount)) {
            return a_source;
        }

        _skinTextureCopies.emplace(a_source, copy);
        REX::DEBUG("Skin TXST {:08X}: {} path(s) redirected", a_source->GetFormID(), a_redirectCount - before);
        return copy;
    }

    RE::BGSTextureSet* RuntimeForms::CloneFaceTextureSet(
        RE::BGSTextureSet* a_source,
        RE::SEX a_sex,
        std::size_t& a_redirectCount)
    {
        if (!a_source) {
            return nullptr;
        }
        if (const auto it = _faceTextureCopies.find(a_source); it != _faceTextureCopies.end()) {
            return it->second;
        }

        if (!CanRedirectTextureSet(a_source, a_sex, true)) {
            return a_source;
        }

        auto* copy = DuplicateTemporary(a_source);
        if (!copy) {
            REX::ERROR("Failed to duplicate face TXST {:08X}", a_source->GetFormID());
            return a_source;
        }

        const auto before = a_redirectCount;
        if (!RedirectTextureSetInPlace(copy, a_sex, true, a_redirectCount)) {
            return a_source;
        }

        _faceTextureCopies.emplace(a_source, copy);
        REX::DEBUG("Face TXST {:08X}: {} path(s) redirected", a_source->GetFormID(), a_redirectCount - before);
        return copy;
    }

    RE::TESObjectARMA* RuntimeForms::CloneArmorAddon(
        RE::TESObjectARMA* a_source,
        RE::SEX a_sex,
        std::size_t& a_redirectCount)
    {
        if (!a_source) {
            return nullptr;
        }

        const auto sex = static_cast<std::size_t>(a_sex == RE::SEX::kFemale ? 1 : 0);
        const auto& cfg = _router.GetConfig();

        if (cfg.verboseLog) {
            const char* model3P = a_source->bipedModel[sex].GetModel();
            const char* model1P = a_source->bipedModel1stPerson[sex].GetModel();
            auto* txst = a_source->skinTextures[sex];
            REX::INFO(
                "ARMA {:08X}: 3P='{}' 1P='{}' skinTXST={:08X}",
                a_source->GetFormID(),
                model3P ? model3P : "",
                model1P ? model1P : "",
                txst ? txst->GetFormID() : 0u);
            if (txst) {
                for (std::size_t i = 0; i < std::size(txst->textures); ++i) {
                    const char* texture = txst->textures[i].textureName.c_str();
                    if (texture && *texture) {
                        REX::INFO("  TXST[{}] '{}'", i, texture);
                    }
                }
            }
        }

        std::optional<std::string> thirdPersonPath;
        std::optional<std::string> firstPersonPath;
        if (cfg.enableSkinMeshes) {
            const char* model = a_source->bipedModel[sex].GetModel();
            if (model && *model) {
                thirdPersonPath = _router.RedirectMesh(model);
            }

            if (cfg.enableFirstPerson) {
                model = a_source->bipedModel1stPerson[sex].GetModel();
                if (model && *model) {
                    firstPersonPath = _router.RedirectMesh(model);
                }
            }
        }

        RE::BGSTextureSet* redirectedTexture = a_source->skinTextures[sex];
        if (cfg.enableSkinTextures && redirectedTexture) {
            redirectedTexture = CloneSkinTextureSet(redirectedTexture, a_sex, a_redirectCount);
        }

        const bool textureChanged = redirectedTexture != a_source->skinTextures[sex];
        if (!thirdPersonPath && !firstPersonPath && !textureChanged) {
            return a_source;
        }

        auto* copy = DuplicateTemporary(a_source);
        if (!copy) {
            REX::ERROR("Failed to duplicate ARMA {:08X}", a_source->GetFormID());
            return a_source;
        }

        if (thirdPersonPath) {
            const std::string source{ a_source->bipedModel[sex].GetModel() };
            copy->bipedModel[sex].SetModel(thirdPersonPath->c_str());
            ++a_redirectCount;
            if (cfg.verboseLog) {
                REX::INFO("Mesh: {} -> {}", source, *thirdPersonPath);
            }
        }
        if (firstPersonPath) {
            const std::string source{ a_source->bipedModel1stPerson[sex].GetModel() };
            copy->bipedModel1stPerson[sex].SetModel(firstPersonPath->c_str());
            ++a_redirectCount;
            if (cfg.verboseLog) {
                REX::INFO("1P mesh: {} -> {}", source, *firstPersonPath);
            }
        }
        if (textureChanged) {
            copy->skinTextures[sex] = redirectedTexture;
        }
        return copy;
    }

    RE::TESObjectARMO* RuntimeForms::BuildUniqueSkin(
        RE::TESObjectARMO* a_source,
        RE::SEX a_sex,
        std::size_t& a_redirectCount)
    {
        if (!a_source) {
            return nullptr;
        }

        if (_router.GetConfig().verboseLog) {
            REX::INFO("Skin ARMO {:08X}: {} ARMA entrie(s)", a_source->GetFormID(), a_source->modelArray.size());
            for (std::size_t i = 0; i < a_source->modelArray.size(); ++i) {
                auto* arma = a_source->modelArray[i].armorAddon;
                REX::INFO("  Skin ARMA[{}] = {:08X}", i, arma ? arma->GetFormID() : 0u);
            }
        }

        std::vector<RE::TESObjectARMA*> replacements;
        replacements.reserve(a_source->modelArray.size());
        bool anyChanged = false;

        for (const auto& entry : a_source->modelArray) {
            auto* replacement = entry.armorAddon ? CloneArmorAddon(entry.armorAddon, a_sex, a_redirectCount) : nullptr;
            replacements.push_back(replacement);
            anyChanged |= replacement && replacement != entry.armorAddon;
        }

        if (!anyChanged) {
            return nullptr;
        }

        auto* copy = DuplicateTemporary(a_source);
        if (!copy) {
            REX::ERROR("Failed to duplicate skin ARMO {:08X}", a_source->GetFormID());
            return nullptr;
        }

        // Ensure this array has its own storage before replacing ARMA pointers.
        copy->modelArray = a_source->modelArray;
        for (std::size_t i = 0; i < copy->modelArray.size() && i < replacements.size(); ++i) {
            if (replacements[i]) {
                copy->modelArray[i].armorAddon = replacements[i];
            }
        }
        return copy;
    }

    bool RuntimeForms::RebuildSkinFromCurrent(RE::SEX a_sex, std::size_t& a_redirectCount)
    {
        if (!_npc) {
            return false;
        }

        _originalNPCSkin = _npc->formSkin;
        _sourceSkin = ResolveSkinSource();
        _uniqueSkin = nullptr;
        _skinTextureCopies.clear();

        if (!_sourceSkin || (!_router.GetConfig().enableSkinMeshes && !_router.GetConfig().enableSkinTextures)) {
            return false;
        }

        _uniqueSkin = BuildUniqueSkin(_sourceSkin, a_sex, a_redirectCount);
        if (_uniqueSkin) {
            _npc->formSkin = _uniqueSkin;
            return true;
        }
        return false;
    }

    bool RuntimeForms::RebuildFaceFromCurrent(RE::SEX a_sex, std::size_t& a_redirectCount)
    {
        if (!_npc || !_router.GetConfig().enableFaceTextures || !_npc->headRelatedData) {
            _faceHeadData = nullptr;
            _originalFaceTexture = nullptr;
            _sourceFaceTexture = nullptr;
            _sourceFaceHeadPartTexture = nullptr;
            _uniqueFaceTexture = nullptr;
            _faceTextureCopies.clear();
            return false;
        }

        _faceTextureCopies.clear();
        _faceHeadData = _npc->headRelatedData;
        _originalFaceTexture = _faceHeadData->faceDetails;
        _sourceFaceHeadPartTexture = GetCurrentFaceHeadPartTexture();
        _sourceFaceTexture = ResolveFaceTextureSource(a_sex);
        _uniqueFaceTexture = nullptr;

        if (_router.GetConfig().verboseLog) {
            REX::INFO(
                "Face sources: explicit={:08X}, winningHeadPartTXST={:08X}, resolved={:08X}",
                _originalFaceTexture ? _originalFaceTexture->GetFormID() : 0u,
                _sourceFaceHeadPartTexture ? _sourceFaceHeadPartTexture->GetFormID() : 0u,
                _sourceFaceTexture ? _sourceFaceTexture->GetFormID() : 0u);
        }

        if (!_sourceFaceTexture) {
            return false;
        }

        auto* unique = CloneFaceTextureSet(_sourceFaceTexture, a_sex, a_redirectCount);
        if (!unique || unique == _sourceFaceTexture) {
            return false;
        }

        _uniqueFaceTexture = unique;
        _faceHeadData->faceDetails = _uniqueFaceTexture;
        return true;
    }

    void RuntimeForms::ResetPlayer3D(bool a_requested, bool a_skinChanged, bool a_faceChanged)
    {
        if (!a_requested || !_player || !_router.GetConfig().reset3DOnApply || (!a_skinChanged && !a_faceChanged)) {
            return;
        }

        // Fallout 4 Actor::Reset3D additional flags. These mirror the targeted skin/face
        // rebuild used by LooksMenu, while deliberately keeping the current head part/mesh.
        // We never request RESET_HEAD or RESET_SKELETON.
        constexpr std::uint32_t kResetModel = 1u << 0;
        constexpr std::uint32_t kResetSkin = 1u << 1;
        constexpr std::uint32_t kResetFace = 1u << 3;
        constexpr std::uint32_t kResetKeepHead = 1u << 10;

        std::uint32_t flags = kResetKeepHead;
        if (a_skinChanged) {
            flags |= kResetModel | kResetSkin;
        }
        if (a_faceChanged) {
            flags |= kResetFace;
        }

        REX::INFO("Requesting targeted player 3D rebuild flags=0x{:X} skin={} face={}",
            flags, a_skinChanged ? "yes" : "no", a_faceChanged ? "yes" : "no");
        _player->Reset3D(false, flags, true, 0);
    }

    bool RuntimeForms::Apply(bool a_reset3D)
    {
        if (_applied && !_suspended) {
            return true;
        }

        _player = RE::PlayerCharacter::GetSingleton();
        _npc = GetPlayerNPC();
        if (!_player || !_npc) {
            REX::WARN("Player or Player NPC is not available yet");
            return false;
        }

        const auto sex = _npc->GetSex();
        std::size_t redirectCount = 0;

        _skinTextureCopies.clear();
        _faceTextureCopies.clear();
        const bool skinChanged = RebuildSkinFromCurrent(sex, redirectCount);
        const bool faceChanged = RebuildFaceFromCurrent(sex, redirectCount);

        _appliedSex = sex;
        _applied = true;
        _suspended = false;
        REX::INFO(
            "Applied player-only runtime redirects: {} path(s), skin={}, face={}",
            redirectCount,
            skinChanged ? "yes" : "no",
            faceChanged ? "yes" : "no");

        ResetPlayer3D(a_reset3D, skinChanged, faceChanged);
        return true;
    }

    bool RuntimeForms::Refresh(bool a_reset3D)
    {
        if (_suspended) {
            return false;
        }
        if (!_applied) {
            return Apply(a_reset3D);
        }

        auto* player = RE::PlayerCharacter::GetSingleton();
        auto* npc = player ? player->GetNPC() : nullptr;
        if (!player || !npc) {
            return false;
        }

        if (player != _player || npc != _npc) {
            Invalidate();
            return Apply(a_reset3D);
        }

        const auto sex = _npc->GetSex();
        if (_appliedSex && *_appliedSex != sex) {
            REX::INFO("Player sex changed; rebuilding player-only runtime forms");
            Invalidate();
            return Apply(a_reset3D);
        }

        std::size_t redirectCount = 0;
        bool skinChanged = false;
        bool faceChanged = false;

        // Another runtime mod may have replaced the player's skin after us. Treat its
        // current winner as the new upstream source and wrap that, rather than fighting
        // it with a stale clone.
        if (_uniqueSkin && _npc->formSkin != _uniqueSkin) {
            skinChanged |= RebuildSkinFromCurrent(sex, redirectCount);
        } else if (!_uniqueSkin && _npc->formSkin != _originalNPCSkin) {
            skinChanged |= RebuildSkinFromCurrent(sex, redirectCount);
        }

        // We never own/replace the HeadRelatedData object. If another mod changes the
        // head-data pointer or its faceDetails TXST (e.g. after LooksMenu closes), its
        // current value becomes our new upstream source. Head parts and head meshes are
        // completely untouched.
        if (_router.GetConfig().enableFaceTextures && _npc->headRelatedData) {
            const bool headDataChanged = _npc->headRelatedData != _faceHeadData;
            const bool faceTextureChanged =
                _faceHeadData == _npc->headRelatedData &&
                _uniqueFaceTexture &&
                _npc->headRelatedData->faceDetails != _uniqueFaceTexture;
            const bool faceWasPreviouslyUnavailable = !_uniqueFaceTexture;

            // If our source originally came from the Face HeadPart (NPC faceDetails was
            // null), a LooksMenu/custom-head change may replace that HeadPart while our
            // explicit unique TXST remains attached. Detect a changed upstream HeadPart
            // TXST, restore the original null pointer, then wrap the new winning TXST.
            const auto* currentHeadPartTexture = GetCurrentFaceHeadPartTexture();
            const bool headPartTextureChangedUnderInheritedSource =
                !headDataChanged &&
                _uniqueFaceTexture &&
                _originalFaceTexture == nullptr &&
                currentHeadPartTexture != _sourceFaceHeadPartTexture;

            if (headPartTextureChangedUnderInheritedSource &&
                _faceHeadData == _npc->headRelatedData &&
                _faceHeadData->faceDetails == _uniqueFaceTexture) {
                _faceHeadData->faceDetails = _originalFaceTexture;
            }

            if (headDataChanged || faceTextureChanged || faceWasPreviouslyUnavailable ||
                headPartTextureChangedUnderInheritedSource) {
                faceChanged |= RebuildFaceFromCurrent(sex, redirectCount);
            }
        }

        const bool changed = skinChanged || faceChanged;
        if (redirectCount > 0 || changed) {
            REX::INFO("Refreshed player redirects after runtime change; {} path(s) redirected", redirectCount);
        }
        ResetPlayer3D(a_reset3D, skinChanged, faceChanged);
        return changed;
    }

    void RuntimeForms::SuspendForSave()
    {
        if (!_applied || _suspended || !_router.GetConfig().saveHygiene || !_npc) {
            return;
        }

        if (_uniqueSkin && _npc->formSkin == _uniqueSkin) {
            _npc->formSkin = _originalNPCSkin;
        }

        if (_faceHeadData && _npc->headRelatedData == _faceHeadData &&
            _uniqueFaceTexture && _faceHeadData->faceDetails == _uniqueFaceTexture) {
            _faceHeadData->faceDetails = _originalFaceTexture;
        }

        _suspended = true;
        REX::DEBUG("Temporarily restored upstream player pointers for save serialization");
    }

    void RuntimeForms::ResumeAfterSave()
    {
        if (!_applied || !_suspended || !_npc) {
            return;
        }

        if (_uniqueSkin && _npc->formSkin == _originalNPCSkin) {
            _npc->formSkin = _uniqueSkin;
        }

        if (_faceHeadData && _npc->headRelatedData == _faceHeadData &&
            _uniqueFaceTexture && _faceHeadData->faceDetails == _originalFaceTexture) {
            _faceHeadData->faceDetails = _uniqueFaceTexture;
        }

        _suspended = false;
        REX::DEBUG("Reattached player-only runtime pointers after save");
    }

    void RuntimeForms::RestoreRuntimePointers()
    {
        if (!_npc) {
            return;
        }

        if (!_suspended && _uniqueSkin && _npc->formSkin == _uniqueSkin) {
            _npc->formSkin = _originalNPCSkin;
        }

        if (!_suspended && _faceHeadData && _npc->headRelatedData == _faceHeadData &&
            _uniqueFaceTexture && _faceHeadData->faceDetails == _uniqueFaceTexture) {
            _faceHeadData->faceDetails = _originalFaceTexture;
        }
    }

    void RuntimeForms::Invalidate()
    {
        RestoreRuntimePointers();

        _npc = nullptr;
        _player = nullptr;
        _originalNPCSkin = nullptr;
        _sourceSkin = nullptr;
        _uniqueSkin = nullptr;
        _faceHeadData = nullptr;
        _originalFaceTexture = nullptr;
        _sourceFaceTexture = nullptr;
        _sourceFaceHeadPartTexture = nullptr;
        _uniqueFaceTexture = nullptr;
        _skinTextureCopies.clear();
        _faceTextureCopies.clear();
        _appliedSex.reset();
        _applied = false;
        _suspended = false;
    }
}
