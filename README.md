# UniquePlayerRedirector 0.2.0

**ESP-less Unique Player-style asset isolation for Fallout 4, implemented as an F4SE/CommonLibF4 runtime plugin.**

The plugin gives the player dedicated body/hand/first-person meshes and dedicated body/hand/face textures while leaving NPCs on whichever assets their normal records/load order provide.

No ESP, ESL or ESM is required.

## Scope

### Included

- third-person skin/body/hand meshes referenced by the player's winning skin ARMA records
- first-person skin/body/hand meshes referenced by those same ARMA records
- ARMA skin TXSTs for body/hands
- player face base TXST (`TESNPC::HeadRelatedData::faceDetails`)
- per-file loose-target fallback
- runtime re-wrapping after another plugin changes the player's skin or face TXST
- pre-save/post-save pointer restoration so temporary forms are not intentionally serialized as the player's permanent skin/face source

### Deliberately not touched

- race records
- animation/behavior data
- head parts
- head NIF path
- FaceGen TRI/morph/sculpt data
- hair/eyes/brows/scars/head rear
- global BSResource path loading

The face subsystem therefore inherits whatever head part/head mesh is already winning. It only substitutes the base face texture set underneath the normal FaceGen/tint pipeline.

## Runtime design

The body path is actor-safe because it never rewrites a global shared form in place:

```text
Player NPC 00000007
    -> temporary cloned skin ARMO
        -> temporary cloned changed ARMA(s)
            -> PlayerCharacterAssets mesh paths
            -> temporary cloned skin TXST(s)
                -> PlayerHumanFemale / PlayerHumanMale texture paths
```

The face path is intentionally even smaller:

```text
Player NPC current HeadRelatedData
    -> faceDetails pointer only
        -> temporary cloned face TXST
            -> PlayerHumanFemale / PlayerHumanMale texture paths
```

`BGSHeadPart`, its model path, and its morph/TRI data are never replaced by this plugin.

## Why this is compatible with a modded/high-poly head

The plugin does not copy or hardcode `FemaleHead.nif` / `MaleHead.nif`. If a head mod or LooksMenu setup wins a different head part/model, that stays in place. UniquePlayerRedirector only changes the player NPC's base face TXST pointer.

If another runtime plugin later changes the player's skin or face TXST, the menu-close compatibility pass treats that new value as the upstream winner and wraps it again. It does not force the original startup record back over the other mod.

## Asset folders

The default paths are the classic Unique Player-style folders:

```text
Data\Meshes\Actors\Character\PlayerCharacterAssets\
Data\Textures\Actors\Character\PlayerHumanFemale\
Data\Textures\Actors\Character\PlayerHumanMale\
```

See `ASSET_LAYOUT.txt` for examples.

## Fallback behavior

`RequireLooseTarget=true` is the safe default. Every individual path is redirected only if its destination file actually exists as a loose file. If you supply only a custom body diffuse, only that diffuse is redirected; missing normals/speculars/meshes keep using their upstream path.

For face TXSTs, `FaceBasenameFallback=true` adds one compatibility rule: if the winning face TXST lives outside `BaseHumanFemale` / `BaseHumanMale`, the plugin tries the same DDS filename under the player-only texture folder. It still redirects only when that destination exists.

## Installation after compiling

1. Build the project on Windows as described in `BUILD.md`.
2. Copy `UniquePlayerRedirector.dll` to `Data\F4SE\Plugins\`.
3. Copy `Data\F4SE\Plugins\UniquePlayerRedirector.ini` beside it.
4. Add your player-only NIF/DDS files under the folders above.
5. Do not enable a Unique Player ESP for this implementation.
6. Launch through F4SE and check the plugin log when testing.

## Status

This package is a **source drop**. The code is written against the current public libxse/CommonLibF4 interfaces and contains no guessed executable addresses or hardcoded relocation IDs of its own.

I could not produce or game-test a Windows DLL in the current Linux environment because the Windows/CommonLibF4 build toolchain is not installed here. Treat the first compiled binary as an experimental build and test on a disposable save first.

## License

GPL-3.0-or-later for this project source. F4SE and CommonLibF4 retain their own licenses.

## No-install cloud build

If you cannot install XMake or Visual Studio locally, use the included `.github/workflows/build-dll.yml` GitHub Actions workflow. See `CLOUD_BUILD.md`.
