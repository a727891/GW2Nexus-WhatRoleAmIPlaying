# Agent plan: BlishHUD module → Nexus addon port

Use this document as a **Cursor agent prompt, rule, or project plan** when porting a
BlishHUD module to a Nexus DLL. Reference implementation:
[NexusRaidClears](https://github.com/soeed/NexusRaidClears) (port of BlishHUD
Clears Tracker).

Copy this file into the **target Blish module repo** (or the new Nexus repo) and
tell the agent:

> Follow `docs/AGENT_BLISH_TO_NEXUS_PORT.md` end-to-end. Use the Blish module as
> behavior reference and NexusRaidClears as engineering reference.

Optional Cursor rule wrapper (`.cursor/rules/blish-to-nexus-port.mdc`):

```yaml
---
description: Port a BlishHUD module to Nexus with static-host bandwidth discipline
alwaysApply: false
---

When porting Blish → Nexus, read and follow docs/AGENT_BLISH_TO_NEXUS_PORT.md.
Reference NexusRaidClears patterns for CMake, manifest sync, asset embedding, and release scripts.
```

---

## Goals

1. **Behavior parity** with the Blish module (panels, options, API usage, persistence).
2. **Single-DLL distribution** for end users where practical (embed static PNGs).
3. **Minimize freesnow static-host bandwidth** — JSON manifest sync only; no redundant PNG pulls.
4. **Identifiable HTTP traffic** — dedicated `User-Agent` on `bhm.blishhud.com` and `assets.gw2dat.com`.
5. **Release-ready build** — static MinGW runtime, stripped export, local deploy scripts.

---

## Inputs the agent needs

| Input | Purpose |
|-------|---------|
| Blish module source (C#) | Behavior, services, settings, UI layout |
| Static data branch/repo (e.g. `bhud-static/<ModuleName>`) | JSON schemas, manifest fields, asset lists |
| Nexus API headers (`extern/nexus/`, `extern/mumble/`) | Addon entry, textures, input binds, data links |
| NexusRaidClears (optional clone) | Proven CMake, embed pipeline, manifest sync, scripts |

Ask the user for:

- Module display name, addon folder name, Nexus signature int
- Static host base URL (`https://bhm.blishhud.com/<Author>.<Module>/static/v2/`)
- Whether dungeons/content are JSON-driven or hardcoded in Blish
- Which Blish textures are still used vs. replaced by ImGui layout

---

## Architecture mapping

| BlishHUD | Nexus port |
|----------|------------|
| `Module.cs` / `LoadAsync` | `src/entry.cpp` (`GetAddonDef`, `AddonLoad`, `AddonRender`, `AddonOptions`) |
| `SettingService` / `.json` persistence | `SettingsStore` + JSON under addon dir |
| `StandardPanel` / XNA `SpriteBatch` | ImGui windows + `ImDrawList` / `AddImage` |
| `ContentsManager` textures | `api->Textures_GetOrCreateFromMemory` (embedded) or `FromFile` (dev fallback) |
| `ModuleMetaDataService.CheckVersions` | `ClearsTrackerManifest` + `ClearsTrackerSync::SyncVersions` |
| `Gw2ApiManager` | `Gw2ApiClient` + `HttpClient` (WinHTTP) |
| `GameService` / `Mumble` | `mumbleLink` via `DL_MUMBLE_LINK` |
| `DownloadTextureService` | **Embed in DLL** or drop if unused — do not re-download manifest PNGs |
| `Logger` | `api->Log(LOGL_*, "AddonName", message)` |
| Background `Task` / async | `std::thread(...).detach()` + `std::atomic` pending flags processed in render tick |

### Required entry-point wiring

```cpp
// entry.cpp pattern
void AddonLoad(AddonAPI_t* api) {
    ImGui::SetCurrentContext(...);
    ImGui::SetAllocatorFunctions(api->ImguiMalloc, api->ImguiFree);
    AppState::Instance().Initialize(api);
    api->GUI_Register(RT_Render, AddonRender);
    api->GUI_Register(RT_OptionsRender, AddonOptions);
    api->InputBinds_RegisterWithString("MOD_TOGGLE", OnToggle, "ALT+SHIFT+...");
}

void AddonRender() {
    state.ProcessPendingStaticDataLoad();  // manifest sync
    state.ProcessPendingApiRefresh();
    // panels...
}
```

---

## Static hosting policy (freesnow bandwidth)

**Design principle:** Each daily user session should cost ~1 KB of manifest traffic
unless data actually changed. Never download PNGs from the manifest unless the UI
still depends on them.

### What to download at runtime

| Resource | Download? | Notes |
|----------|-----------|-------|
| `clears_tracker.json` | **Every boot** | ~1 KB; drives cache bust |
| Versioned JSON (`raid_data.json`, etc.) | **Only when stale** | Compare manifest stamps vs local manifest |
| Manifest `assets` PNGs | **No** | Embed or remove from UI |
| Manifest `gridbox_masks` PNGs | **No** | Embed in DLL |
| `assets.gw2dat.com` icons | **On demand, cached** | Per asset ID; not freesnow static host |

### Manifest cache bust (must port from Blish)

Mirror `ModuleMetaDataService.CheckVersions`:

1. Download remote `clears_tracker.json`.
2. Load local cached manifest.
3. For each version field, if `remote != local` **or** `cache_bust == true` → download that JSON file only.
4. Save remote manifest locally (updates stamps + MOTD).
5. Reload in-memory parsers from disk cache.

Typical field → file mapping (adjust per module):

| Manifest key | Cached filename |
|--------------|-----------------|
| `raid_data` | `raid_data.json` |
| `strike_data` | `strike_data.json` |
| `daily_bounties` | `daily_bounties.json` |
| `fractal_map_data` | `fractal_maps.json` |
| `fractal_instabilities` | `fractal_instabilities.json` |

Boot flow:

1. **Sync:** load JSON from disk immediately (fast UI).
2. **Async:** manifest sync in background thread.
3. **Reload:** if any file updated, re-parse and rebuild UI groups; refresh API poll.

If manifest fetch fails, keep cache; on first install fall back to `LoadOrDownload` for missing core files.

### Bandwidth budget (order-of-magnitude)

Assume 1000 DAU, one session/day:

| Scenario | Static host traffic |
|----------|---------------------|
| Returning users, no data publish | ~1000 × 1 KB ≈ **1 MB/day** |
| 5% fresh installs | +50 × ~220 KB ≈ **+11 MB/day** |
| Publish raid_data bump | +1000 × ~30 KB ≈ **+30 MB one-time** |

Embedding PNGs avoids ~1.4 MB per fresh install that older Blish/Nexus ports pulled from manifest.

### HTTP User-Agent

Centralize in `Branding.h` + `HttpClient`:

```cpp
inline constexpr const char* kHttpUserAgent = "ModuleName-Nexus/1.0.0";
```

Set on requests to `bhm.blishhud.com` and `assets.gw2dat.com` only (not `api.guildwars2.com`).

---

## Asset embedding strategy

**End-user release = single DLL.** Build-time embed all small PNGs the module needs.

### Pipeline (copy from NexusRaidClears)

1. `assets/logos/`, `assets/textures/`, `assets/textures/masks/` — source PNGs.
2. `scripts/embed_assets.py` — generates `build/generated/Embedded*.cpp/h` byte arrays.
3. `CMakeLists.txt` — custom command before compile; glob PNG deps for rebuild.
4. Runtime — `Textures_GetOrCreateFromMemory(identifier, data, size)`.
5. Dev fallback — `Textures_GetOrCreateFromFile` from addon dir when embed load fails.

### Embed vs download decision tree

```
Is the PNG needed at runtime?
├─ No  → remove Blish DownloadTextureService usage entirely
├─ Yes → is it large/decorative/optional?
│   ├─ Yes → simplify UI (ImGui flat layout) instead of downloading
│   └─ No  → embed in DLL
```

### DLL size expectations

| Component | Approx size |
|-----------|-------------|
| Stripped release DLL | ~5 MB |
| Unstripped (debug sections) | ~18 MB — strip before release |
| Embedded PNGs | Usually < 1 MB total |

Release script must run `x86_64-w64-mingw32-strip --strip-debug` on export.

---

## Implementation phases

### Phase 0 — Discovery

- [ ] Read Blish `Module.cs` load order and service graph.
- [ ] Inventory static JSON files and `clears_tracker.json` fields.
- [ ] List all HTTP endpoints (static host, gw2dat, GW2 API).
- [ ] Map Blish panels → planned ImGui panels.
- [ ] Identify hardcoded C# data vs JSON-driven data.

### Phase 1 — Scaffold

- [ ] Create repo: `CMakeLists.txt`, MinGW toolchain, `extern/nexus`, `extern/mumble`.
- [ ] `src/entry.cpp` with `GetAddonDef`, load/render/options/unload.
- [ ] `AppState` singleton (settings, services, mutex for shared data).
- [ ] `StaticDataLoader` (cache dir, download, load cached, write cached).
- [ ] `HttpClient` with User-Agent policy.
- [ ] Static link MinGW runtime (`-static-libgcc -static-libstdc++ -static`).
- [ ] Verify imports: `objdump -p build/*.dll` — no `libstdc++-6.dll`.

### Phase 2 — Data layer

- [ ] Port JSON parsers (`FromJson` per static file).
- [ ] Port `ClearsTrackerManifest` + `ClearsTrackerSync` (or module-specific manifest type).
- [ ] Wire `AppState::Initialize` cache-first + `RequestStaticDataLoad` every boot.
- [ ] Port GW2 API client + polling interval setting.
- [ ] Port account/key storage if module uses API keys.

### Phase 3 — Core services

Port Blish services to C++ one at a time; keep names recognizable:

- [ ] Clears fetch services (raids, strikes, fractals, dungeons, …)
- [ ] Reset watcher (daily/weekly)
- [ ] Map watcher (if map-based clears)
- [ ] Persistence files under `clearsTracker/` or addon dir
- [ ] Mentor/progress/daily bounty aux services as needed

### Phase 4 — UI

- [ ] Overlay panels (grid layout, colors, tooltips, screen clamp).
- [ ] Options panel with sidebar sections matching Blish settings tabs.
- [ ] `OptionsUiKit` — shared headings, checkboxes, color pickers, expansion rows.
- [ ] `ContentLogoService` — embedded expansion logos (if selection UI uses them).
- [ ] `GridMaskService` — embedded organic cell masks.
- [ ] `QuickAccessService` — corner icon + context menu + keybind toggle.
- [ ] `DatAssetIconService` — gw2dat icons with disk cache (not static host).

### Phase 5 — Assets & build scripts

- [ ] `scripts/embed_assets.py` for module PNGs.
- [ ] `scripts/build-release.sh` — configure, build, strip → `dist/*.dll`.
- [ ] `scripts/deploy-to-gw2.sh` — `--release`, `--ftue` flags for local testing.
- [ ] `.gitignore` — `build/`, `dist/`, `*.dll`.

### Phase 6 — Documentation & polish

- [ ] `README.md` — install (single DLL), API key, build, deploy, troubleshooting.
- [ ] `CONTRIBUTING.md` — static JSON workflow, what needs code vs data-only changes.
- [ ] `CHANGELOG.md`, `LICENSE`.
- [ ] `Branding.h` — display name, version, User-Agent, patch notes URL.
- [ ] Update static-host docs to note Nexus no longer downloads manifest PNGs.

### Phase 7 — Verification

- [ ] Cross-compile Release build succeeds.
- [ ] Stripped DLL loads in Nexus (no missing MinGW DLLs).
- [ ] FTUE: delete `clearsTracker/` → manifest sync populates JSON.
- [ ] Cache hit: second launch → only manifest GET (~1 KB).
- [ ] Bump manifest version → only affected JSON re-downloads.
- [ ] `cache_bust: true` → all JSON files refresh.
- [ ] MOTD update propagates without full data reload when versions unchanged.
- [ ] Options, panels, tooltips, keybind, corner icon parity spot-check vs Blish.

---

## Code conventions (match NexusRaidClears)

- C++20, namespace short alias (`rc`, `mod`, etc.).
- Header/source split under `src/{core,data,services,ui}/`.
- Minimal diff scope — port behavior, don't redesign.
- Comments only for non-obvious manifest/sync/threading logic.
- No tests unless requested.
- Do not commit unless user asks.

### CMake dependencies (typical)

- **imgui** — UI
- **nlohmann_json** — static data + settings
- **winhttp, ws2_32, crypt32, shell32** — HTTP/TLS on Windows

### Persistence layout

```
<GW2>/addons/<AddonName>/
  settings.json
  api_accounts.json          # if multi-key
  raid_settings.json         # per-module visibility/labels
  clearsTracker/             # static JSON cache (manifest + data files)
  textures/assets/           # gw2dat icon cache only
```

---

## Common pitfalls

| Pitfall | Fix |
|---------|-----|
| Nexus won't load DLL | Static-link libgcc/libstdc++/winpthread |
| 18 MB DLL | Strip debug sections in release script |
| Users never get data updates | Port manifest `CheckVersions`; don't cache-forever without sync |
| Manifest sync SIGPIPE crash | Never pipe `cmake` into `awk` under `set -o pipefail` in shell scripts |
| Re-downloading all JSON every boot | Compare version stamps; only fetch stale files |
| Downloading manifest PNGs | Embed or delete UI dependency |
| `assets` host 404 | JSON uses `static/v2/`; old PNG path was `static/` without v2 |
| Blish namespace mismatch in entry | Keep `AddonRender`/`AddonOptions` in same linkage unit as registration |
| GW2 API rate limits | Respect poll interval setting; batch fetches |

---

## Agent execution notes

When running this plan:

1. **Read Blish module first** — do not invent behavior.
2. **Clone patterns from NexusRaidClears** — manifest sync, embed script, build-release, HttpClient UA.
3. **Prefer static JSON over C++** for encounter tables when Blish does.
4. **Ask before** changing freesnow hosting contracts or publishing static data.
5. **Report bandwidth impact** when adding new runtime downloads.
6. Build and run `build-release.sh` before declaring done.

---

## Reference files (NexusRaidClears)

| Concern | Path |
|---------|------|
| Addon entry | `src/entry.cpp` |
| Manifest sync | `src/data/ClearsTrackerManifest.{h,cpp}` |
| Boot/cache flow | `src/core/AppState.cpp` |
| Static HTTP | `src/data/StaticDataLoader.{h,cpp}`, `src/services/HttpClient.cpp` |
| User-Agent | `src/core/Branding.h` |
| Asset embed | `scripts/embed_assets.py`, `build/generated/Embedded*.cpp` |
| Embedded masks | `src/ui/GridMaskService.cpp` |
| Embedded logos | `src/ui/ContentLogoService.cpp` |
| Corner icon | `src/ui/QuickAccessService.cpp` |
| Release build | `scripts/build-release.sh` |
| Local deploy | `scripts/deploy-to-gw2.sh` |
| MinGW toolchain | `cmake/mingw-w64-toolchain.cmake` |
| Maintainer guide | `CONTRIBUTING.md` |

---

## Customization per module

```text
MODULE_NAME       = WhatRoleAmIPlaying
ADDON_DIR         = NexusWhatAmIPlaying
NAMESPACE         = wap
SIGNATURE         = -2024061501
STATIC_HOST       = https://bhm.blishhud.com/Soeed.WhatRoleAmIPlaying/roles.json
USER_AGENT        = WhatAmIPlaying-Nexus/1.0.0
CACHE_DIR         = whatAmIPlaying
BLISH_REPO        = WhatAmIPlaying (WhatRoleAmIPlayingModule.cs)
STATIC_DATA_REPO  = WhatAmIPlaying-static (branch bhud-static/Soeed.WhatRoleAmIPlaying)
```

**This repo specifics:** no embedded PNGs (gw2dat asset IDs only), single JSON sync file,
corner icon + main window UI, minimal settings (`LastShownMotdId`, corner icon toggle).
Elite spec unlock filtering matches Blish dev mode (all specs treated as unlocked).

---

*Derived from the NexusRaidClears port (BlishHUD Clears Tracker → Nexus).*
