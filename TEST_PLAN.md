# In-game validation plan

Use a disposable save for the first compiled binary.

## A. Body/NPC isolation

1. Disable/remove the old Unique Player ESP.
2. Put a very obvious player-only body NIF in `PlayerCharacterAssets`.
3. Load a save next to a human NPC using the same sex/body family.
4. Expected: player uses the unique mesh; NPC remains on its normal mesh.
5. Repeat with one body DDS only.
6. Expected: only that supplied map redirects; missing player-only maps fall back cleanly.

## B. Hands and first person

1. Add unique hand NIF/DDS files.
2. Verify third-person hands.
3. Add first-person body/hand NIF files if the upstream skin ARMA actually references them.
4. Verify first-person and third-person independently.

## C. Face base textures

1. Start with `EnableFaceTextures=true` and `FaceBasenameFallback=true`.
2. Add visibly different player-only head diffuse/normal/specular files.
3. Load the save and inspect the face in neutral lighting.
4. Expected: player face uses the unique base maps; NPC faces do not.
5. Verify existing tint/makeup/sculpt still appears.

## D. Custom/high-poly head compatibility

1. Install/enable the custom head mod first and confirm its head shape works without this plugin.
2. Enable UniquePlayerRedirector without changing the head mod.
3. Expected: head geometry stays identical; only player face textures change.
4. The log should show `Face texture:` entries but no head NIF rewrite exists in this project.

## E. LooksMenu/runtime override ordering

1. Load with both LooksMenu and UniquePlayerRedirector.
2. Enter LooksMenu and make a visible face/skin-related change.
3. Exit the menu.
4. Expected: the selected head/sculpt remains; UniquePlayerRedirector re-wraps the current winning face TXST to the player-only DDS paths.
5. Check the log for `Refreshed player redirects after runtime change` only when an upstream pointer actually changed.

## F. Save/load hygiene

1. Save with the plugin active.
2. Load that save normally.
3. Quit, disable only the DLL (leave normal game assets intact), then load the disposable save again.
4. The save should not require a UniquePlayerRedirector ESP because none exists; the source pointers are restored during save serialization by design.

## G. Failure isolation

If the face pipeline is problematic, set:

```ini
EnableFaceTextures=false
```

Body/hands/first-person routing should continue independently.
