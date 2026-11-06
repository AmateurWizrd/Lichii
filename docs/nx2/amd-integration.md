# Experimental AMD branch

This branch starts at `oot-next` commit `7a473ed1` and takes the requested fixes from `chuppatricky-amd-testbuild-v13-source`. It targets Windows with AMD's proprietary Vulkan driver. It does not establish that these changes fix Ocarina of Time on an RX 7800 XT; that needs a run on the affected hardware.

## Integration decisions

- `69d1fe67` and `72e79fa0` are excluded: both need `db56da6c`, which adds the absent OoT visual-workaround header, tests and pipeline wiring. No unlisted fog prerequisite is imported.
- `72b116da` and `df06d106` expected the later listed `981ee335` compile logger. Their logging additions were deferred until that logger was picked, then restored with the unoptimized labels. No outside logging prerequisite is imported.
- `e5b9708d` keeps the back-quiet handshake and scheduler ownership checks. Its idle-hook conflict is resolved without `SplitIdleHook`, a profiler zone absent from this base. The existing `SplitWait` scope remains.
- The query-report series `27bee9bf`, `42832d88`, `d5771ec7` is retained. Conditional rendering looks up `delayed_reports`, including the occlusion HLE's condition. `SemiFlushRegion` also recognizes overlapping delayed reports so #215's `FlushQueryResults` and the HLE's back-thread `QueryDma::FlushPending` wait for their completion callbacks. It does not publish a finalized report early, since the report must remain behind preceding host GPU work. Delayed reports stay out of ordinary CPU dirty-region tracking, preserving nonblocking progress polling.
- `2c6ca549` changes the preflush default, while keeping #191's `DeferredDownloadTraps`, dirty-region check, trap rearming, and download batching. `EDEN_NX2_PREFLUSH=0` selects trap-and-flush. `EDEN_NX2_PREFLUSH_EVERY` still controls batching, and `EDEN_NX2_PREFLUSH_DOWNLOADS=0` still disables fence downloads and forces reactive reads even on pages marked preflushable. These controls affect different parts of the path and are not aliases.
- `6aaf70f4` adds the fragment output mask without importing the unrelated `zero_texture_handles` member that appeared in its conflict context.
- `3f17618d` recomputes LSFG availability before reporting it. The old later availability check is removed so switching from unavailable LSFG to the built-in generator is not stuck behind the old state.

## Main-branch candidates

`97c4aab7` is device-independent: gathered multi-range entries now track their source buffers and retired buffers drain each frame. This also applies on NVIDIA when a range needs gathering rather than sparse binding.

`e5b9708d` fixes split-thread scheduler ownership rather than an AMD-specific condition. `62d18eab` makes CAS retry data depend on the failed CAS's returned value. Both are candidates for the NVIDIA branch, subject to its normal testing.

`4689a160` is relevant to any device without dynamic blend state. `6aaf70f4` masks attachments the fragment shader does not write on every vendor. Their benefits on NVIDIA hardware have not been measured here.
