# Aligned allocator hang investigation

The allocator changes that rounded the alignment prefix up to a 16-byte
boundary introduce a new failure mode when the free block we pick is just
large enough to satisfy the request before rounding.

* `block_supports_aligned_request` now normalizes the padding and rejects
  candidates that would leave `header + minimum_payload` or less space for the
  aligned block.
* `split_block` refuses to create a prefix whenever the leftover bytes are
  less than or equal to another block header plus the minimum 16-byte payload,
  returning the original block unchanged.

When the normalized padding pushes the leftover space down to exactly
`header + minimum_payload`, the first split is rejected even though the helper
claimed the block was usable. The caller then advances to `prefix_block->next`
(which is either `nullptr` or an unrelated neighbor), so the aligned request
fails despite sufficient space or corrupts the free list, which matches the
hang seen around the aligned allocator tests.

## Suggested fix

Teach `block_supports_aligned_request` to mirror the split logic instead of
only checking for `remaining_size >= aligned_size`. After subtracting the
normalized padding, require more than `sizeof(Block) + minimum_payload` before
declaring the block usable. When that extra space is not available, the helper
now rejects the candidate so the caller keeps searching instead of attempting a
split that is guaranteed to fail. Aligning both helpers on the same minimum-size
contract prevents the prefix split from returning the original block and keeps
the free-list traversal from walking into `nullptr`.
