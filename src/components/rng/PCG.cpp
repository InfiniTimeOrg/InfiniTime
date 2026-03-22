#include "components/rng/PCG.h"

using namespace Pinetime::Controllers;

RNG::RNG(result_type value) {
  // pcg32_srandom_r(&rng, 0x853c49e6748fea9bULL, value);
  rng.state = 0U;
  rng.inc = (value << 1u) | 1u;
  (*this)(); // pcg32_random_r(rng);
  rng.state += 0x853c49e6748fea9bULL;
  (*this)(); // pcg32_random_r(rng);
};

RNG& RNG::operator=(const pcg32_random_t& other) {
  rng.state = other.state;
  rng.inc = other.inc | 1;
  return *this;
};

template <class SeedSeq>
RNG::RNG(SeedSeq& seq) {
  seed(seq);
};

/*
RNG::RNG(const RNG& other) {
  RNG tmp = other;
  RNG::result_type* ptr = (RNG::result_type*) this;
  tmp.generate(ptr, ptr + (sizeof(RNG) / sizeof(RNG::result_type)));
};
*/
template <typename SeedSeq>
void RNG::seed(SeedSeq& seq) {
  RNG::result_type* ptr = (RNG::result_type*) this;
  seq.generate(ptr, ptr + (sizeof(RNG) / sizeof(RNG::result_type)));
  rng.inc |= 1;
};

void RNG::seed(RNG& other) {
  RNG::result_type* ptr = (RNG::result_type*) this;
  other.generate(ptr, ptr + (sizeof(RNG) / sizeof(RNG::result_type)));
  rng.inc |= 1;
};

RNG::result_type RNG::operator()() {
  // return pcg32_random_r(&rng);
  uint64_t oldstate = rng.state;
  rng.state = oldstate * 6364136223846793005ULL + rng.inc;
  uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
  uint32_t rot = oldstate >> 59u;
  return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
};

RNG::result_type RNG::operator()(RNG::result_type bound) {
  // return pcg32_boundedrand_r(&rng, bounds);
  //   To avoid bias, we need to make the range of the RNG a multiple of
  //  bound, which we do by dropping output less than a threshold.
  //  A naive scheme to calculate the threshold would be to do
  //
  //      uint32_t threshold = 0x100000000ull % bound;
  //
  //  but 64-bit div/mod is slower than 32-bit div/mod (especially on
  //  32-bit platforms).  In essence, we do
  //
  //      uint32_t threshold = (0x100000000ull-bound) % bound;
  //
  //  because this version will calculate the same modulus, but the LHS
  //  value is less than 2^32.

  uint32_t threshold = -bound % bound;

  // Uniformity guarantees that this loop will terminate.  In practice, it
  // should usually terminate quickly; on average (assuming all bounds are
  // equally likely), 82.25% of the time, we can expect it to require just
  // one iteration.  In the worst case, someone passes a bound of 2^31 + 1
  // (i.e., 2147483649), which invalidates almost 50% of the range.  In
  // practice, bounds are typically small and only a tiny amount of the range
  // is eliminated.
  for (;;) {
    uint32_t r = (*this)(); // pcg32_random_r(rng);
    if (r >= threshold)
      return r % bound;
  }
};

// std::seed_seq interface
template <typename It>
void RNG::generate(It start, It end) {
  for (; start != end; ++start)
    *start = (*this)();
};

std::size_t RNG::size() const noexcept {
  return sizeof(rng) / sizeof(RNG::result_type);
};

template <class OutputIt>
void RNG::param(OutputIt dest) const {
  std::size_t i = 0;
  const std::size_t n = size();
  RNG::result_type* ptr = (RNG::result_type*) this;
  for (; i < n; ++i, ++dest, ++ptr) {
    *dest = ptr;
  }
};