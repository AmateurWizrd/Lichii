<!--
# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: 2018 yuzu Emulator Project
# SPDX-License-Identifier: GPL-2.0-or-later
-->

<p align="center">
  <img src="dist/branding/lichii_icon_with_logo.png" alt="Lichii" width="600">
</p>

# About this fork

This fork tunes Lichii for The Legend of Zelda: Ocarina of Time on one PC: Ryzen 7 7800X3D, GeForce RTX 5070 Ti, Windows 11. The work is done with AI coding agents (Claude, Codex and others) and checked by playing the game. Measurements and logs go back upstream as comments on the matching issues and pull requests.

Branches:

- `oot-play`: upstream `nx2` with the open fixes #149, #204, #215 and #216 merged. This is the build we play on.
- `oot-hle`: `oot-play` plus a replacement for the game's occlusion test macro (`EDEN_NX2_OCCLUSION_HLE=1`, off by default). The macro read two visibility counters from GPU memory up to 45,000 times a second and made the first GPU thread wait for the second each time; the replacement hands the comparison to Lichii's render-enable condition instead. Experimental.
- `oot-161`: `oot-play` with #161 (GPU thread synchronization) and the occlusion replacement, for measurement. Experimental.
- `oot-next`: upstream `nx2` after #194 (the Frizti port) with #204/#215, #216 and #149's remaining shader fix, plus everything from `oot-hle`: the occlusion replacement, `EDEN_NX2_WRITER_WAIT_LEARN=1` (skip the per-entry wait for queued compute/DMA work on pages that never held GPU-written commands), `EDEN_NX2_IMAGE_POOL=1` (reuse deleted images' VkImages), and a texture cache collector cap that grows with the resolution scale. Experimental.

Settings that work on this PC: GPU Mode Low (Lichii runs its second GPU thread only on Low), Docked, 2x resolution, Area filter. Use the MSVC build: the Windows MinGW (PGO) build has no AV1 decoder, and the game's intro movies need one.

No game files, keys or firmware are in this repository. The upstream README follows.

# Lichii

Lichii is an experimental, open-source Nintendo Switch and Switch 2 emulator written in C++. It builds on Eglantine, Eden, yuzu and Sudachi, preserving their contributions while developing this fork's game compatibility and renderer fixes.

This is an independent fork, not an official release of those upstream projects. Lichii is not affiliated with Nintendo.

## Downloads and setup

Published builds belong on this fork's [releases page](https://github.com/zortos293/Eglantine/releases). The repository still uses its historical Eglantine name; the desktop application is now Lichii.

On Windows, extract the entire build archive and run **lichii.exe**. Only use games and system files you have legally obtained. Games, keys and firmware are not included.

### Existing saves and settings

The rebrand intentionally retains the existing Eden/Eglantine data directories and configuration keys. It does not move or delete saves, settings or shader caches. Back up your saves before testing experimental builds.

## Compatibility and issue reports

Compatibility and performance vary by game, hardware and driver. Rendering errors, stutters and crashes remain possible. Passing automated tests does not establish that a game renders correctly, and this project makes no universal frame-rate guarantee.

When [reporting an issue](https://github.com/zortos293/Eglantine/issues), include:

- The exact build, game version, CPU, GPU and driver version.
- Relevant settings and steps to reproduce the problem.
- A screenshot and the log from the affected session, after checking it for personal information.

Do not upload game files, keys, firmware or private account information.

## Building and contributing

See the [build guide](docs/Build.md), [user documentation](docs/user/README.md) and [coding guidelines](docs/Coding.md). Some inherited documentation and internal build target names still use upstream names. The GUI build target remains `yuzu`; its output is `lichii.exe` on Windows and `lichii` on other desktop platforms.

Keep changes focused and distinguish code-level checks from actual gameplay testing.

## Artwork
Art created by YOB (yobslab)
The supplied Lichii logo, icon and library artwork are in [dist/branding](dist/branding). Windows, Linux and macOS icon resources are generated from that artwork by [Generate-LichiiIcons.ps1](tools/Generate-LichiiIcons.ps1).

## Support Lichii

Lichii is free and stays free. If you want to help, you can donate in crypto. Donations help pay for the AI tools and build machines used to develop this open-source project faster.

**Solana (SOL)**
```text
JepEjfyWjGjhJvHSrqj52KtLjcbbbmUh1iGqerYBF4w
```

**Ethereum (ETH)**
```text
0x055ABa1b80Bc35C836Ce7E9F47c2b3d6C75b5534
```

**Bitcoin (BTC)**
```text
bc1qcvxdpf4qvdhqp8ee4s355cjmxrzsphsdwpl8w4
```

**BNB Smart Chain (BNB)**
```text
0x055ABa1b80Bc35C836Ce7E9F47c2b3d6C75b5534
```

**Polygon**
```text
0x055ABa1b80Bc35C836Ce7E9F47c2b3d6C75b5534
```

Send each coin on its matching network. Coins sent on the wrong network can't be recovered.

> [!NOTE]
> Donations are voluntary gifts and are non-refundable. A donation does not buy any product, service, perk, early access, exclusive build, priority support or say in the project. Every release stays free and public for everyone.

## Credits and license

Lichii inherits the work of the Eglantine, Eden, yuzu and Sudachi projects and their contributors, including Eglantine's original developer Camille LaVey. Original copyright notices and third-party licenses are retained.

See [LICENSE.txt](LICENSE.txt) and the source files for applicable license terms.
