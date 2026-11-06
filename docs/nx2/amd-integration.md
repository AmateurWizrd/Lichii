# Experimental AMD branch

This branch starts at `main` commit `738e4520` and takes the remaining AMD-only fixes from `chuppatricky-amd-testbuild-v13-source`. It targets Windows with AMD's proprietary Vulkan driver. It does not establish that these changes fix Ocarina of Time on an RX 7800 XT; that needs a run on the affected hardware.

## Integration decisions

- `69d1fe67` and `72e79fa0` are excluded: both need `db56da6c`, which adds the absent OoT visual-workaround header, tests and pipeline wiring. No unlisted fog prerequisite is imported.
- `72b116da` and `df06d106` expected the later listed `981ee335` compile logger. Their logging additions were deferred until that logger was picked, then restored with the unoptimized labels. No outside logging prerequisite is imported.
- Compute safe compilation keeps upstream's `BuildsUnoptimized` hash exceptions in auto mode, including on newer AMD proprietary drivers. The old-driver size rule adds to those exceptions. `EDEN_NX2_AMD_SAFE_COMPILE=0` disables both; `=1` explicitly uses the configurable size rule on any AMD proprietary driver. The log uses the same combined decision as the pipeline flag.
- `e5b9708d` keeps the back-quiet handshake and scheduler ownership checks. Its idle-hook conflict is resolved without `SplitIdleHook`, a profiler zone absent from this base. The existing `SplitWait` scope remains.
- The query-report series `27bee9bf`, `42832d88`, `d5771ec7` is retained. Conditional rendering looks up `delayed_reports`, including the occlusion HLE's condition. `SemiFlushRegion` also recognizes overlapping delayed reports so #215's `FlushQueryResults` and the HLE's back-thread `QueryDma::FlushPending` wait for their completion callbacks. It does not publish a finalized report early, since the report must remain behind preceding host GPU work. Delayed reports stay out of ordinary CPU dirty-region tracking, preserving nonblocking progress polling.
- `2c6ca549` changes the preflush default inside main's `forced_preflush` structure: `forced_preflush.value_or(true)`. It keeps #191's `DeferredDownloadTraps`, dirty-region check, trap rearming, and download batching. `EDEN_NX2_PREFLUSH=0` selects trap-and-flush. `EDEN_NX2_PREFLUSH_EVERY` still controls batching, and `EDEN_NX2_PREFLUSH_DOWNLOADS=0` still disables fence downloads and forces reactive reads even on pages marked preflushable. These controls affect different parts of the path and are not aliases.
- `6aaf70f4` adds the fragment output mask without importing the unrelated `zero_texture_handles` member that appeared in its conflict context.
- `3f17618d` recomputes LSFG availability before reporting it. The old later availability check is removed so switching from unavailable LSFG to the built-in generator is not stuck behind the old state.

## Picks now supplied by main

- `07802030` (original `62d18eab`) is dropped. `68caa6c7` feeds the returned compare-exchange word into both `CasLoop` and `DefineWriteStorageCasLoopFunction` retries. It also fixes the shared-store retries and shader typing.
- `429d197d` (original `92d8eec9`) is dropped. Upstream #295 supplies the same `VectorViewIsExact` predicate and uses it for all four 64/128-bit storage load/store paths.
- `4e57f825` (original `39cdadb4`) is dropped. Upstream #295 supplies global vector word accesses when `min_ssbo_alignment` is below the access size and the expanded buffer/index callback. Main intentionally retains the no-descriptor-aliasing fallback that searches per word, with explicit coverage in `spirv_storage_alignment.cpp`. The older AMD pick would also collapse that fallback into a single search; the replay keeps upstream's implementation and tests.
- Base repair picks `40af18b8`, `df62a22a` and `bc3cb520` are not replayed: main already contains their source commits `594bbdf3`, `1d455af1` and `71c6b3d7`.
- README picks `780f9824` and `19d2ae89` are not replayed: main already contains the fork/branch documentation.

The upstream merge also supplies compute write ordering, and main includes `fb164aa3`'s `robustBufferAccess2` fix. Neither is replaced by the AMD replay.

## Main-branch candidates

`97c4aab7` is device-independent: gathered multi-range entries now track their source buffers and retired buffers drain each frame. This also applies on NVIDIA when a range needs gathering rather than sparse binding.

`e5b9708d` fixes split-thread scheduler ownership rather than an AMD-specific condition. It is a candidate for the NVIDIA branch, subject to its normal testing. The CAS retry correction is already supplied by main's upstream merge.

`4689a160` is relevant to any device without dynamic blend state. `6aaf70f4` masks attachments the fragment shader does not write on every vendor. Their benefits on NVIDIA hardware have not been measured here.

