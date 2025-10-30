# RNG Statistical Quality Guarantees

This document summarizes the statistical health checks that guard the pseudo-random
number generators provided by `RNG/rng.hpp` and explains how to combine stream
seeds safely when running workloads in parallel.

## Distribution conformance checks

The automated test suite now includes a `test_rng_statistical_quality` group that
verifies two critical properties:

- **Uniform chi-squared test** &mdash; draws 20,000 samples from `ft_random_int`,
  folds them into ten equally sized buckets, and rejects the sequence when the
  chi-squared statistic exceeds the 0.99 critical value for nine degrees of
  freedom (21.666).  This protects against regressions that bias the uniform
  engine toward a subset of the available integers.
- **Normal Kolmogorov&ndash;Smirnov test** &mdash; generates 4,096 samples with
  `ft_random_normal`, sorts them, and compares the empirical CDF to the standard
  normal CDF.  The test fails when the maximum deviation exceeds 0.05, which is
  tighter than the 0.01 significance band for the sample size.  This ensures the
  Box&ndash;Muller based path stays within expected tolerance when updating the
  implementation or the underlying engine.

Both checks are deterministic by seeding the shared engine with
`ft_rng_test_seed_engine`, so CI failures are actionable while still exercising
meaningful statistical thresholds.

## Guidance for combining RNG streams

When spawning parallel workers or sharding simulations, each worker must receive
an independent seed stream to avoid correlated results.  The library exposes the
following helpers that make this trivial:

1. Call `ft_random_seed` (or `rng_secure_uint64`) once to obtain a base seed for
   the job.
2. Pass the base seed and a unique stream identifier (thread index, task ID, or
   similar) to `rng_stream_seed`.  The function hashes the pair through
   `rng_stream_split.cpp`, yielding a decorrelated seed that is safe to pass to
   `ft_seed_random_engine`.
3. For bulk initialization, use `rng_stream_seed_sequence` (or the string based
   counterparts) so each worker receives a consistent, independent subsequence.

Never reuse the same `(base_seed, stream_identifier)` pair across workers, and
avoid manual arithmetic on the raw base seed &mdash; XOR or addition alone cannot
remove correlations introduced by the underlying engine.  The provided helpers
mirror the jump-ahead logic used internally, so every stream advances through a
unique portion of the generator period without overlap.
