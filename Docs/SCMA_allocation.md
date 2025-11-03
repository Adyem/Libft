# SCMA Allocation and Heap Compaction

The SCMA module maintains two primary data structures:

* `g_scma_heap`: an `ft_vector<unsigned char>` that stores the raw bytes for all
  managed allocations contiguously in a single growable buffer.
* `g_scma_blocks`: an `ft_vector<scma_block>` where each entry tracks a
  block's byte `offset` into `g_scma_heap`, its current `size`, an `in_use`
  flag, and a `generation` counter that is embedded in the public
  `scma_handle` to detect stale references.

## Allocation Flow

When `scma_allocate` is called, the allocator first ensures that the module is
initialized and that the requested `size` is non-zero. It then calls
`scma_compact` so that any gaps created by previously freed blocks are removed
before evaluating free space (`SCMA/scma_core.cpp`, `scma_allocate`). The
required total size is computed as the sum of the current used heap space and
the requested block size, and `scma_ensure_capacity` grows `g_scma_heap` if the
current capacity would be exceeded.

Next, the allocator scans `g_scma_blocks` looking for a slot whose `in_use` flag
is clear. If none is found, a new `scma_block` entry is appended. Otherwise, the
available slot is reused. In either case, the block's `offset` is set to the
current end of the live heap region, `size` is recorded, `in_use` is set to 1,
and the block's `generation` is incremented (wrapping away from
`FT_SYSTEM_SIZE_MAX`). The handle returned to the caller stores the block's
index in `g_scma_blocks` and the corresponding `generation`, letting the runtime
reject handles that point to recycled slots.

Because every live block is written to the active end of the heap, allocating a
new block after compaction always yields a contiguous layout with no holes.

## Heap Reorganization

Heap defragmentation happens inside `scma_compact`, which walks every block in
`g_scma_blocks` and keeps a running `new_offset`. For each block whose `in_use`
flag is set, the function checks whether its recorded `offset` matches the
current `new_offset`. If the values differ, the live bytes are moved in-place by
`std::memmove` so they immediately follow the previously compacted block. The
block's `offset` is then updated, and `new_offset` advances by the block's
`size`. When the pass completes, the allocator's global `g_scma_used_size`
tracks the end of the compacted data and therefore the next free byte in the
heap.

The compactor is invoked in three situations:

1. **Before each allocation** (`scma_allocate`): ensures the allocator sees a
   contiguous live region before checking capacity.
2. **Immediately after freeing a block** (`scma_free`): removes the newly
   freed bytes from the middle of the heap.
3. **Midway through resizing** (`scma_resize`): after copying the old block's
   bytes into a temporary buffer and marking it free, the allocator compacts the
   heap so the resized block can be reissued at the tail without gaps.

Because compaction keeps `g_scma_heap` densely packed, the allocator can move
blocks while they are in use. Clients never receive raw pointers to the heap;
they interact exclusively via handles. Every API entry point validates the
handle by comparing the stored `index` and `generation` against the current
block metadata. When a block slot is recycled, its `generation` changes, so
outstanding handles that refer to the previous incarnation of the slot are
rejected. This indirection lets the runtime safely relocate blocks without
breaking clients.

## Resizing

`scma_resize` combines these behaviors. It first validates the handle, then
copies the block's existing bytes into a temporary heap allocation. After
marking the block as free and compacting, the allocator treats the resize as a
fresh allocation: the block is reattached at the current end of the heap, its
`generation` is incremented, and `g_scma_used_size` grows to include the new
length. Finally, the preserved bytes are copied back—truncated if the block was
shrunk. Throughout the process the caller's handle remains valid because the
slot index stays the same and the generation updates atomically when the new
block is installed.

Together, these mechanisms let SCMA maintain a defragmented heap at runtime
while providing a safe handle-based API for accessing relocatable memory.

## Typed Access Helpers

To simplify structured access without exposing raw heap pointers, the module
now ships with the `scma_handle_accessor<T>` template. It wraps a
`scma_handle`, re-validates it before each operation, and requires that `T` be
trivially copyable so instances can be marshaled through the relocation layer.
The helper offers:

* `read_struct` / `write_struct` to copy an entire struct-sized payload.
* `read_at` / `write_at` for indexed access into arrays of `T` stored within a
  block.
* `get_count` to report how many `T` elements fit in the allocation.

Each method updates `ft_errno` and the accessor's internal `_error_code`, so
callers can query failure reasons via `get_error` and `get_error_str` while the
allocator continues compacting blocks transparently in the background.
