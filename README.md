# What Role Am I Playing? (Nexus)

Nexus addon port of the [BlishHUD What Role Am I Playing?](https://github.com/a727891/BlishHud-WhatRoleAmIPlaying) module.

Suggest a random raid build to play next based on a role category you pick - DPS, boon DPS, healer, or full random.

## Features

- Random role suggestions from the shared Snow Crows–derived build library (`roles.json`)
- Role filters: Full Random, DPS (Power/Condi), Boon DPS (Quickness/Alacrity), Healer variants
- Main window with button grid and result panel (profession/elite/boon icons, description, Snow Crows link)
- Corner icon with context menu for quick picks
- Background sync of `roles.json` from `bhm.blishhud.com` with `last_updated` cache checks
- gw2dat icon cache for profession, elite spec, boon, and corner icons

## Requirements

- [Nexus](https://raidcore.gg/gw2/nexus) in Guild Wars 2
- Windows x64 at runtime (DLL is cross-compiled from Linux)

## Installation

1. Copy `NexusWhatAmIPlaying.dll` to `<GW2>/addons/` (directly in `addons/`, not a subfolder)
2. Launch GW2 with Nexus enabled
3. Enable **What Role Am I Playing?** in Nexus addon settings
4. Use the corner icon or **Open What Am I Playing?** on the addon page

On first run the addon downloads `roles.json` into `<GW2>/addons/NexusWhatAmIPlaying/whatAmIPlaying/`.

## Build & deploy

### Toolchain (Fedora/Nobara)

```bash
sudo dnf install mingw64-gcc-c++ mingw64-winpthreads-static cmake ninja git python3
```

### Build

```bash
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Stripped release export:

```bash
./scripts/build-release.sh
```

Deploy locally:

```bash
./scripts/deploy-to-gw2.sh --release
./scripts/deploy-to-gw2.sh --release --ftue   # clear cache to test first-load sync
```

## Static data

Role builds are fetched from:

`https://bhm.blishhud.com/Soeed.WhatRoleAmIPlaying/roles.json`

UI icons load from `https://assets.gw2dat.com/` with `User-Agent: WhatAmIPlaying-Nexus/1.0.0`.

To update the build library, edit the static branch in [WhatAmIPlaying-static](https://github.com/a727891/BlishHud-WhatRoleAmIPlaying/tree/bhud-static/Soeed.WhatRoleAmIPlaying) and bump `last_updated`.

Thank you to **Freesnöw** for continued hosting of the role library and gw2dat icon CDN used by this addon.

## Manual test checklist

- [ ] Addon loads without Nexus errors
- [ ] Corner icon appears after gw2dat icon download
- [ ] Main window opens from corner icon and Nexus config button
- [ ] Random / DPS / Healer buttons return a role with icons and description
- [ ] **View Build** opens Snow Crows URL in browser
- [ ] Context menu quick picks work
- [ ] Second launch skips full re-download when `last_updated` unchanged
- [ ] `--ftue` deploy clears cache and re-syncs on next boot

## Credits

- **Soeed** - BlishHUD module and Nexus port
- **Freesnöw** - continued static data and config file hosting (`bhm.blishhud.com`, `assets.gw2dat.com`)
- **Raidcore** - Nexus addon platform

## License

MIT - see [LICENSE](LICENSE).
