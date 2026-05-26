# RNG

The `RNG` module provides global random helpers, secure-random byte helpers, deterministic stream seeding, distribution functions, vectorized sampling, and template containers for decks and loot tables.

## Global Random Functions

- `ft_random_int()` - Returns a pseudo-random integer.
- `ft_dice_roll(int32_t number, int32_t faces)` - Rolls `number` dice with `faces` sides and returns the total.
- `ft_random_float()` - Returns a pseudo-random float.
- `ft_random_normal()` - Samples a normal distribution.
- `ft_random_exponential(float lambda_value)` - Samples an exponential distribution.
- `ft_random_poisson(double lambda_value)` - Samples a Poisson distribution.
- `ft_random_binomial(int32_t trial_count, double success_probability)` - Samples a binomial distribution.
- `ft_random_geometric(double success_probability)` - Samples a geometric distribution.
- `ft_random_gamma(float shape, float scale)` - Samples a gamma distribution.
- `ft_random_beta(float alpha, float beta)` - Samples a beta distribution.
- `ft_random_chi_squared(float degrees_of_freedom)` - Samples a chi-squared distribution.
- `ft_random_seed(const char *seed_string)` - Seeds the global generator from optional text and returns the seed.
- `rng_seed_value(const char *seed_string)` / `rng_seed_value(const ft_string &seed_string)` - Derives a deterministic numeric seed.
- `ft_random_uint32()` - Returns a pseudo-random `uint32_t`.
- `ft_generate_uuid(char output_buffer[37])` - Writes a UUID string into a 37-byte caller buffer.

## Secure Random and Streams

- `rng_secure_bytes(unsigned char *buffer, ft_size_t length)` - Fills a buffer from the OS secure random source.
- `rng_secure_bytes_with_fallback(unsigned char *buffer, ft_size_t length, int32_t *fallback_used)` - Fills secure bytes and reports whether fallback randomness was needed.
- `rng_secure_uint64(uint64_t *value, int32_t *fallback_used)` - Generates a secure 64-bit value.
- `rng_secure_uint32(uint32_t *value, int32_t *fallback_used)` - Generates a secure 32-bit value.
- `rng_stream_seed(uint64_t base_seed, uint64_t stream_identifier, uint64_t *stream_seed)` - Derives an independent stream seed.
- `rng_stream_seed_sequence(...)` - Fills a seed sequence buffer for a stream.
- `rng_stream_seed_from_string(...)` - Derives a stream seed from text.
- `rng_stream_seed_sequence_from_string(...)` - Fills a seed sequence from text.

## Distribution Utilities

- `rng_gamma_pdf(...)` / `rng_gamma_cdf(...)` - Evaluate gamma density and cumulative distribution.
- `rng_beta_pdf(...)` / `rng_beta_cdf(...)` - Evaluate beta density and cumulative distribution.
- `rng_chi_squared_pdf(...)` / `rng_chi_squared_cdf(...)` - Evaluate chi-squared density and cumulative distribution.
- `ft_random_int_vector(...)` - Fills an array with random integers in a range.
- `ft_random_float_vector(...)` - Fills an array with random floats.

## `rng_stream`

- Lifecycle: `rng_stream()`, `~rng_stream()`, `initialize()`, copy/move initialization, `destroy()`, and `move(...)`.
- Thread safety: `enable_thread_safety()`, `disable_thread_safety()`, and `is_thread_safe()`.
- Seeding: `reseed(uint32_t seed_value)` and `reseed_from_string(const char *seed_string)`.
- Sampling: `random_int()`, `dice_roll(...)`, `random_float()`, `random_normal()`, `random_exponential(...)`, `random_poisson(...)`, `random_binomial(...)`, `random_geometric(...)`, `random_gamma(...)`, `random_beta(...)`, and `random_chi_squared(...)`. Each returns a `Pair` containing status and sampled value.

## Decks and Loot Tables

- `ft_deck<ElementType>` - A lifecycle deck backed by `ft_vector<ElementType *>`.
- `ft_deck::pop_random_element()` - Removes and returns a random element.
- `ft_deck::get_random_element()` - Returns a random element without removing it.
- `ft_deck::shuffle()` - Randomizes deck order.
- `ft_deck::draw_top_element()` - Removes and returns the top element.
- `ft_deck::peek_top_element()` - Returns the top element without removing it.
- `ft_loot_entry<ElementType>` - Stores one item pointer with weight and rarity.
- `ft_loot_entry` setters/getters - Set or retrieve item pointer, base weight, rarity, and effective weight.
- `ft_loot_table<ElementType>` - Owns weighted loot entries.
- `ft_loot_table::add_element(...)` - Adds an item with weight and rarity.
- `ft_loot_table::get_random_loot()` - Selects a weighted item without removing it.
- `ft_loot_table::pop_random_loot()` - Selects and removes a weighted item.
- `ft_loot_table::size()` - Returns the number of loot entries.
