# Contributing

Thanks for helping maintain **What Role Am I Playing?** for Nexus. Community role builds are shared between Blish and Nexus clients via JSON hosted by Freesnöw (`bhm.blishhud.com`).

## Development setup

See [README.md](README.md) for toolchain, build, and deploy commands.

### Project layout

| Path | Purpose |
|------|---------|
| `src/core/` | App state, settings, branding |
| `src/data/` | `roles.json` models and JSON parsing |
| `src/services/` | HTTP, role config sync, role selection |
| `src/ui/` | Main window, corner icon, gw2dat icons, Nexus config entry |
| `extern/nexus/` | Nexus addon API headers |
| `extern/mumble/` | Mumble link header |

## Static data

Community role builds live in the `bhud-static/Soeed.WhatRoleAmIPlaying` branch (often cloned as `WhatAmIPlaying-static`). Bump `last_updated` when editing `roles.json` so clients re-download.

The addon caches under `<GW2>/addons/WhatAmIPlaying/whatAmIPlaying/`.

## Code changes

Treat the Blish C# module (`WhatRoleAmIPlaying/`) as behavior reference. Match role filter logic in `RoleConfigService` and selection flow in `MainWindow`.
