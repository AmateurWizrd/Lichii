// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdlib>

namespace VideoCommon::Nx2Perf {

// Read once at static initialization. A function-local static would pay MSVC's thread-safe init
// check (a TLS load and compare) on every call, and this is asked for every 3D method (~1% of the
// GPU back thread in Ocarina of Time).
inline const bool verify_enabled = [] {
    const char* value = std::getenv("EDEN_NX2_PERF_VERIFY");
    return value != nullptr && value[0] == '1';
}();

inline bool Verify() {
    return verify_enabled;
}

} // namespace VideoCommon::Nx2Perf
