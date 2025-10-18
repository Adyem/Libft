# Compression tuning guide

The streaming helpers expose knobs that allow callers to trade compression ratio for throughput when working with large payloads. These controls live on `t_compress_stream_options` and apply to both `ft_compress_stream_with_options` and the higher level convenience wrappers.

## Buffer sizing

- `input_buffer_size` and `output_buffer_size` determine how much data the streaming loop reads or writes per iteration.
- Leave these fields at `0` to use the historical 4 KiB defaults, or set them explicitly to match the I/O profile of the application.
- Larger buffers improve throughput by reducing syscalls and allow the compressor to see more data at once, which helps when pairing with large sliding windows.

## Sliding window and memory levels

- `window_bits` configures the deflate sliding window. Valid values are between 8 (256 bytes) and 15 (32 KiB). Smaller windows consume less memory but reduce the compressor's ability to find long-distance matches.
- `memory_level` controls the size of the internal hash tables. Valid values span 1 through 9. Higher levels cost more memory but reduce hash collisions and usually improve ratios.
- `compression_level` mirrors zlib's numeric levels (`Z_NO_COMPRESSION` through `Z_BEST_COMPRESSION`). The library keeps the previous behaviour by defaulting to `Z_BEST_COMPRESSION` when callers do not provide options.
- `strategy` exposes zlib's strategy constants. Leaving this at `Z_DEFAULT_STRATEGY` suits general data; other strategies are available for specialized workloads (e.g., `Z_RLE` for run-length encoded content).

All values are validated before compression or decompression begins. If a setting falls outside the supported range, the functions return `FT_ERR_INVALID_ARGUMENT` so callers can fall back to safer presets.

## Presets

The library now ships two ready-to-use presets:

- `ft_compress_stream_apply_speed_preset` selects 16 KiB buffers, uses `Z_BEST_SPEED`, shrinks the window to 13 bits (or the minimum 8-bit window when the platform requires it), and lowers the memory level to favor throughput on latency-sensitive workloads.
- `ft_compress_stream_apply_ratio_preset` expands the buffers to 32 KiB, keeps a full 15-bit window, bumps the memory level toward the maximum, and sticks with `Z_BEST_COMPRESSION` for offline or archival jobs.

Both helpers only touch the tuning-related fields so that existing callbacks remain intact. Callers can pick a preset, adjust individual knobs, and then invoke the streaming APIs.

## When to adjust presets

- Choose the speed preset for interactive services where latency matters more than achieving the smallest output size.
- Pick the ratio preset when compressing backups or other payloads where CPU time is less important than disk footprint.
- Override `window_bits` or `memory_level` manually when hardware constraints demand tighter bounds. The validation pass guarantees these tweaks never put zlib into undefined territory.
