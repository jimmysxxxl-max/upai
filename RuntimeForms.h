#pragma once

#include "AssetRouter.h"

namespace UPR
{
    class RuntimeForms
    {
    public:
        explicit RuntimeForms(AssetRouter a_router);

        bool Apply(bool a_reset3D);
        bool Refresh(bool a_reset3D);
        void SuspendForSave();
        void ResumeAfterSave();
        void Invalidate();

        [[nodiscard]] bool IsApplied() const noexcept { return _applied; }
        [[nodiscard]] bool IsSuspended() const noexcept { return _suspended; }
        [[nodiscard]] bool WantsMenuRefresh() const noexcept { return _router.GetConfig().reapplyAfterMenuClose; }

    private:
        template <class T>
        static T* DuplicateTemporary(T* a_source)
        {
            if (!a_source) {
                return nullptr;
            }
            auto* form = a_source->CreateDuplicateForm(false, nullptr);
            auto* copy = form ? form->As<T>() : nullptr;
            if (copy) {
                copy->SetTemporary();
            }
            return copy;
        }

        RE::TESNPC* GetPlayerNPC() const;
        RE::TESObjectARMO* ResolveSkinSource() const;
        RE::BGSTextureSet* GetCurrentFaceHeadPartTexture() const;
        RE::BGSTextureSet* ResolveFaceTextureSource(RE::SEX a_sex) const;

        RE::BGSTextureSet* CloneSkinTextureSet(RE::BGSTextureSet* a_source, RE::SEX a_sex, std::size_t& a_redirectCount);
        RE::BGSTextureSet* CloneFaceTextureSet(RE::BGSTextureSet* a_source, RE::SEX a_sex, std::size_t& a_redirectCount);
        RE::TESObjectARMA* CloneArmorAddon(RE::TESObjectARMA* a_source, RE::SEX a_sex, std::size_t& a_redirectCount);
        RE::TESObjectARMO* BuildUniqueSkin(RE::TESObjectARMO* a_source, RE::SEX a_sex, std::size_t& a_redirectCount);

        bool CanRedirectTextureSet(RE::BGSTextureSet* a_set, RE::SEX a_sex, bool a_face) const;
        bool RedirectTextureSetInPlace(
            RE::BGSTextureSet* a_set,
            RE::SEX a_sex,
            bool a_face,
            std::size_t& a_redirectCount);
        bool RebuildSkinFromCurrent(RE::SEX a_sex, std::size_t& a_redirectCount);
        bool RebuildFaceFromCurrent(RE::SEX a_sex, std::size_t& a_redirectCount);
        void RestoreRuntimePointers();
        void ResetPlayer3D(bool a_requested, bool a_skinChanged, bool a_faceChanged);

        AssetRouter _router;
        RE::TESNPC* _npc{ nullptr };
        RE::PlayerCharacter* _player{ nullptr };

        // Exact NPC skin pointer before our substitution. This can legitimately be null;
        // ResolveSkinSource() then falls back to the current race's skin.
        RE::TESObjectARMO* _originalNPCSkin{ nullptr };
        RE::TESObjectARMO* _sourceSkin{ nullptr };
        RE::TESObjectARMO* _uniqueSkin{ nullptr };

        // We do not replace HeadRelatedData or any BGSHeadPart. Only faceDetails is
        // substituted on whichever HeadRelatedData object is current when we apply.
        RE::TESNPC::HeadRelatedData* _faceHeadData{ nullptr };
        // Exact faceDetails pointer that was present before our substitution. It may be
        // null even when the race supplies a default face-details TXST.
        RE::BGSTextureSet* _originalFaceTexture{ nullptr };
        RE::BGSTextureSet* _sourceFaceTexture{ nullptr };
        RE::BGSTextureSet* _sourceFaceHeadPartTexture{ nullptr };
        RE::BGSTextureSet* _uniqueFaceTexture{ nullptr };

        std::unordered_map<RE::BGSTextureSet*, RE::BGSTextureSet*> _skinTextureCopies;
        std::unordered_map<RE::BGSTextureSet*, RE::BGSTextureSet*> _faceTextureCopies;

        std::optional<RE::SEX> _appliedSex;
        bool _applied{ false };
        bool _suspended{ false };
    };
}
