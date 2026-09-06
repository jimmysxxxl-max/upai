# Technical notes

## 1. Why there is no global BSResource hook

The generic resource loader sees paths, not a reliable "this request belongs to PlayerCharacter" identity. Global path rewriting would therefore risk sharing a player-only path through normal resource caching.

UniquePlayerRedirector instead gives the player temporary runtime forms whose model/TXST paths are already unique before the resource layer resolves them.

## 2. Body/hands/first-person path

The player's effective skin is resolved as:

1. `TESNPC::formSkin`, when present.
2. Otherwise the current race's `formSkin`.

The source skin ARMO is duplicated at runtime. Only ARMA entries that actually have at least one redirectable asset are replaced with temporary ARMA duplicates.

For the player's sex, the ARMA clone can change:

- `bipedModel[sex]` (third-person)
- `bipedModel1stPerson[sex]` (first-person)
- `skinTextures[sex]` (TXST pointer)

The facebone model arrays are intentionally not touched.

## 3. Face path and custom-head compatibility

The face subsystem does **not** duplicate or replace `BGSHeadPart` and does not touch its model/morph members.

Instead, it duplicates the current face-details TXST and assigns that duplicate only to:

```text
Player TESNPC -> HeadRelatedData -> faceDetails
```

That is why a high-poly/custom head mesh remains whichever head mod/load order/runtime plugin selected it.

The source TXST is normally the current `headRelatedData->faceDetails`. If that pointer is null, the code can use the current race's default face-details TXST as the *source for cloning* while still remembering that the exact upstream `faceDetails` pointer was null. Save/load restoration therefore returns the exact pointer state it found rather than making the race fallback explicit on the NPC. The plugin does not create a new HeadRelatedData object when none exists; it simply skips face redirection in that unusual state.

## 4. Face basename compatibility fallback

A normal face redirect maps:

```text
Actors\Character\BaseHumanFemale\SomeFile.dds
-> Actors\Character\PlayerHumanFemale\SomeFile.dds
```

If the winning face TXST instead points at something like:

```text
Actors\Character\SomeHeadMod\MyHead_d.dds
```

and `FaceBasenameFallback=true`, the plugin also tries:

```text
Actors\Character\PlayerHumanFemale\MyHead_d.dds
```

With `RequireLooseTarget=true`, that mapping exists only when the destination file is actually present.

## 5. TXST resource IDs

After a cloned TXST texture path changes, its matching `BSResource::ID` is regenerated with `GenerateFromPath()`. This prevents a duplicate TXST from retaining the source file ID after its string path has been changed.

## 6. Runtime coexistence / "hook after it" behavior

Application is queued through the F4SE task interface rather than mutating the forms directly inside the F4SE message callback.

A UI menu-close sink then performs a cheap compatibility check. If another runtime mod replaced the player's skin or changed the current face-details TXST, that current value becomes the new upstream source and UniquePlayerRedirector builds a fresh player-only wrapper around it.

Crucially, a head-part/head-mesh change by itself requires no action from this plugin because the plugin never owned that data in the first place.

## 7. Save hygiene

Runtime-cloned TESForms are marked temporary. On the F4SE pre-save message, the plugin temporarily swaps the player back to the exact upstream NPC skin pointer and upstream face TXST pointer. On post-save it reattaches the runtime-only pointers without forcing a 3D rebuild.

Before loading another save, runtime state is restored/invalidated and rebuilt from the newly loaded player state.

## 8. What this source intentionally avoids

- no hardcoded Fallout executable offsets
- no custom trampoline hooks
- no BSResource loader detour
- no modification of shared NPC skin/ARMA/TXST records in place
- no head-part replacement
- no head model rewrite
- no race mutation
- no animation/behavior mutation

The only shared forms used as inputs are duplicated before their mesh/TXST paths are changed.

## 9. Alpha lifetime limitation

Temporary duplicate forms are engine-owned after `CreateDuplicateForm()`/`SetTemporary()`. If another runtime mod repeatedly replaces the player's skin or face TXST in one session, this plugin may create additional temporary wrappers. It stops referencing older wrappers but does not attempt to manually free engine-owned TESForms. This should be profiled during extended compatibility testing.

## 10. Loose assets by default

`RequireLooseTarget=true` uses `std::filesystem::exists()` under `Data\Meshes` / `Data\Textures`. This is intentional fail-safe behavior and does not query BSResource/BA2 contents. The first alpha should therefore be tested with loose Unique Player assets.
