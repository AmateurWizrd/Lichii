// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <span>
#include <vector>
#include <catch2/catch_test_macros.hpp>

#include "video_core/buffer_cache/uniform_stream_reuse.h"

namespace {
void Check(bool condition, const char* message) {
    INFO(message);
    REQUIRE(condition);
}

struct Memory {
    std::array<u8, 8192> bytes{};
    std::array<bool, 2> mapped{true, true};
    size_t unsafe_reads{};
    template <typename T>
    T* GetPointer(u64 address) {
        if (address >= bytes.size() || !mapped[address / Core::DEVICE_PAGESIZE]) {
            return nullptr;
        }
        return reinterpret_cast<T*>(bytes.data() + address);
    }
    void ReadBlockUnsafe(u64 address, void* destination, u32 size) {
        ++unsafe_reads;
        Check(address + size <= bytes.size(), "read outside guest memory");
        auto* output = static_cast<u8*>(destination);
        for (u32 i = 0; i < size; ++i) {
            const u64 current = address + i;
            output[i] = mapped[current / Core::DEVICE_PAGESIZE] ? bytes[current] : 0;
        }
    }
};

struct Runtime {
    struct StreamUniformRef {
        size_t allocation;
    };
    std::vector<std::vector<u8>> allocations;
    StreamUniformRef last{}, bound{};
    u64 tick{1};
    u64 CurrentTick() const {
        return tick;
    }
    StreamUniformRef LastStreamUniformRef() const {
        return last;
    }
    std::span<u8> BindMappedUniformBuffer(size_t, u32, u32 size) {
        allocations.emplace_back(size);
        last = bound = StreamUniformRef{allocations.size() - 1};
        return allocations.back();
    }
    void RebindStreamUniform(StreamUniformRef ref, u32 size) {
        Check(ref.allocation < allocations.size(), "reused a recycled allocation");
        Check(allocations[ref.allocation].size() == size, "reused wrong allocation size");
        bound = ref;
    }
    void NextTick() {
        ++tick;
        allocations.clear();
    }
};

void CheckBound(const Runtime& runtime, const Memory& memory, u64 address, u32 size) {
    const auto& bytes = runtime.allocations.at(runtime.bound.allocation);
    Check(bytes.size() == size &&
              std::memcmp(bytes.data(), memory.bytes.data() + address, size) == 0,
          "bound uniform data differs from guest memory");
}
} // namespace

TEST_CASE("Uniform stream reuse preserves data through mutation and session resets",
          "[video_core]") {
    using VideoCommon::BindSmallUniformReusing;
    Memory memory;
    Runtime runtime;
    VideoCommon::ResetUniformStreamReuse<Runtime>();
    auto bind = [&](size_t stage, u32 slot, u64 address, u32 size) {
        Check(BindSmallUniformReusing(runtime, memory, stage, slot, slot, address, size),
              "valid small uniform rejected");
        CheckBound(runtime, memory, address, size);
    };
    bind(0, 0, 0, 64);
    bind(0, 0, 0, 64);
    Check(runtime.allocations.size() == 1, "identical bytes were copied twice");
    memory.bytes[7] = 23;
    bind(0, 0, 0, 64);
    Check(runtime.allocations.size() == 2, "changed bytes reused stale data");
    bind(0, 1, 0, 64);
    bind(1, 0, 0, 64);
    Check(runtime.allocations.size() == 4, "stage or slot shared the wrong allocation");
    runtime.NextTick();
    bind(0, 0, 0, 64);
    Check(runtime.allocations.size() == 1, "a new command buffer reused an old allocation");
    VideoCommon::ResetUniformStreamReuse<Runtime>();
    bind(0, 0, 0, 64);
    Check(runtime.allocations.size() == 2, "session reset retained an old allocation");
    bind(4, 17, 0, 4096);
    const auto before = runtime.allocations.size();
    for (auto [stage, slot, size] :
         {std::array<u32, 3>{5, 0, 64}, {0, 18, 64}, {0, 0, 0}, {0, 0, 4097}}) {
        Check(!BindSmallUniformReusing(runtime, memory, stage, slot, slot, 0, size),
              "invalid uniform range did not use the fallback");
    }
    Check(runtime.allocations.size() == before, "fallback allocated stream memory");

    u32 random = 17;
    for (u32 index = 0; index < 10000; ++index) {
        random = random * 1664525U + 1013904223U;
        if (index % 37 == 0)
            runtime.NextTick();
        if (index % 503 == 0)
            VideoCommon::ResetUniformStreamReuse<Runtime>();
        const u32 size = 1 + random % 4096;
        const u64 address = (random >> 12) % (memory.bytes.size() - 4096);
        if (index % 3 == 0)
            memory.bytes[address + random % size] ^= u8{0x5a};
        const size_t stage = (random >> 20) % 5;
        const u32 slot = (random >> 24) % 18;
        bind(stage, slot, address, size);
        bind(stage, slot, address, size);
    }
}

TEST_CASE("Uniform stream reuse uses only mapped single-page guest spans", "[video_core]") {
    Memory memory;
    Runtime runtime;
    VideoCommon::ResetUniformStreamReuse<Runtime>();
    auto bind = [&](u32 slot, u64 address, u32 size) {
        Check(VideoCommon::BindSmallUniformReusing(runtime, memory, 0, slot, slot, address, size),
              "valid small uniform rejected");
    };

    bind(0, Core::DEVICE_PAGESIZE - 64, 64);
    Check(memory.unsafe_reads == 0, "page-ending range missed the direct guest path");
    bind(1, Core::DEVICE_PAGESIZE - 63, 64);
    Check(memory.unsafe_reads == 1, "cross-page range did not use ReadBlockUnsafe");
    const auto& crossed = runtime.allocations.at(runtime.bound.allocation);
    Check(std::memcmp(crossed.data(), memory.bytes.data() + Core::DEVICE_PAGESIZE - 63, 64) == 0,
          "cross-page fallback changed guest bytes");

    memory.mapped[1] = false;
    bind(2, Core::DEVICE_PAGESIZE, 64);
    Check(memory.unsafe_reads == 2, "unmapped page did not use ReadBlockUnsafe");
    const auto& unmapped = runtime.allocations.at(runtime.bound.allocation);
    Check(std::all_of(unmapped.begin(), unmapped.end(), [](u8 byte) { return byte == 0; }),
          "unmapped range did not preserve zero-fill behavior");
}
