<!--
# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: 2018 yuzu Emulator Project
# SPDX-License-Identifier: GPL-2.0-or-later
-->

<p align="center">
  <img src="dist/branding/lichii_icon_with_logo.png" alt="Lichii" width="600">
</p>

# lichii-OoTPerf

A Lichii fork focused on The Legend of Zelda: Ocarina of Time (the Switch 2 release) for now: performance, rendering fixes and game patches. It is tuned and measured on one PC (Ryzen 7 7800X3D, GeForce RTX 5070 Ti, Windows 11) but runs on any GPU Lichii supports. The work is done with AI coding agents (Claude, Codex and others) and checked by playing the game. Measurements and logs go back upstream as comments on the matching issues and pull requests.

Branches:

- `main`: the build to play. Upstream `nx2` (with #194, the Frizti port, #240, Dark's Dynamic FPS, and #295's storage access and compute ordering fixes) plus the open fixes #204/#215, #216 and #149's remaining shader fix, and this fork's own work: `EDEN_NX2_PREFLUSH=1` (copy back at fences the buffers the game reads, instead of draining the GPU for each read: 17.6 -> 16.7 ms at Kokiri Forest, the game's 60 fps cap), the occlusion test macro replacement (`EDEN_NX2_OCCLUSION_HLE=1`), `EDEN_NX2_WRITER_WAIT_LEARN=1` (skip the per-entry wait for queued compute/DMA work on pages that never held GPU-written commands; a page's first GPU write is only detected after it was read), `EDEN_NX2_IMAGE_POOL=1` (reuse deleted images' VkImages; helps only when the texture cache evicts), a texture cache collector cap that grows with the resolution scale, and the real-time speed patch correcting the game's speed from 3% below 60 fps. The switches are off by default. It also carries fixes from other forks: vouion's deferred-clear, TXD/ATOMG register and fence-wait fixes and #191's batched fence downloads (`EDEN_NX2_PREFLUSH_EVERY=N`, default 1), mja00's depth-compare, sampler-compare and `robustBufferAccess2` fixes, Frizti's usage-tracker, LayeredFS-for-NUP and config-array fixes. Real-time speed and Dark's Dynamic FPS hook the same instruction: with both enabled in Add-Ons, Real-time speed applies; disable it to use Dark's.
- `amd`: `main` plus AMD fixes collected in chuppatricky's AMD test builds: unoptimized builds of the shaders old AMD drivers miscompile, misaligned storage and global access splits, CAS retry, NaN-defined min/max/clamp, blend and color-mask fixes for drivers without dynamic blend, query report timing, one wait per occlusion query resolve, preflush by default, and GPU breadcrumbs that log device loss. The status bar shows the display frame rate. For Radeon cards. Experimental: it builds and passes the tests, but nobody has run it on an AMD card yet. Integration notes: `docs/nx2/amd-integration.md`.
- `oot-play`, `oot-hle`, `oot-161`: earlier stages, kept for reference. `oot-hle` introduced the occlusion replacement: the game's macro read two visibility counters from GPU memory up to 45,000 times a second and made the first GPU thread wait for the second each time; the replacement hands the comparison to Lichii's render-enable condition.

Settings that work on this PC: GPU Mode Low (Lichii runs its second GPU thread only on Low), Docked, 2x resolution, Area filter, and `EDEN_NX2_PREFLUSH=1` plus `EDEN_NX2_OCCLUSION_HLE=1` set in the environment. Build with MSVC (the Windows MinGW build only gained AV1 for the intro movies in #284).

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
