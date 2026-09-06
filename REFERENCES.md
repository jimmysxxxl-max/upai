# API references used by the source design

These are upstream code references for maintainers, not runtime dependencies beyond F4SE/CommonLibF4.

- CommonLibF4 `TESObjectARMA`: exposes per-sex third-person models, facebone models, first-person models and skin TXST pointers.
  - https://github.com/libxse/commonlibf4/blob/main/include/RE/T/TESObjectARMA.h
- CommonLibF4 `TESObjectARMO`: exposes the skin armor's ARMA model array.
  - https://github.com/libxse/commonlibf4/blob/main/include/RE/T/TESObjectARMO.h
- CommonLibF4 `TESNPC`: exposes `HeadRelatedData::faceDetails` and the current head-part list.
  - https://github.com/libxse/commonlibf4/blob/main/include/RE/T/TESNPC.h
- CommonLibF4 `BGSTextureSet`: exposes the eight texture strings and matching resource IDs.
  - https://github.com/libxse/commonlibf4/blob/main/include/RE/B/BGSTextureSet.h
- CommonLibF4 F4SE interfaces: messaging and task queue APIs.
  - https://github.com/libxse/commonlibf4/blob/main/include/F4SE/Interfaces.h
- CommonLibF4 UI event source: menu-open/close sink registration.
  - https://github.com/libxse/commonlibf4/blob/main/include/RE/U/UI.h
- LooksMenu/F4EE `SkinInterface.cpp`: historical proof that a face texture override can assign the NPC face-texture pointer and request a face rebuild independently from changing the head part.
  - https://github.com/expired6978/F4SEPlugins/blob/master/f4ee/SkinInterface.cpp
