#pragma once
#include <cstdint>
#include <FreeRTOS.h>
#include <timers.h>
#include "components/motion/MotionController.h"

namespace Pinetime {
  namespace Controllers {
    /*
     * PCG Random Number Generation for C.
     *
     * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
     * See the License for the specific language governing permissions and
     * limitations under the License.
     *
     * For additional information about the PCG random number generation scheme,
     * including its license and other licensing options, visit
     *
     *     http://www.pcg-random.org
     */

    struct pcg_state_setseq_64 {              // Internals are *Private*.
      uint64_t state = 0x853c49e6748fea9bULL; // RNG state.  All values are possible.
      uint64_t inc = 0xda3e39cb94b95bdbULL;   // Controls which RNG sequence (stream) is
                                              // selected. Must *always* be odd.
      // pcg_state_setseq_64() = default;
      // pcg_state_setseq_64(uint64_t s, uint64_t i);
    };
    typedef struct pcg_state_setseq_64 pcg32_random_t;

    struct RNG {
      pcg32_random_t rng = {};
      // <random> interface
      using result_type = uint32_t;

      static constexpr result_type min() {
        return result_type {};
      };

      static constexpr result_type max() {
        return ~result_type {0UL};
      };

      // Constructors
      // RNG();
      RNG() = default;
      explicit RNG(result_type value);

      template <class SeedSeq>
      explicit RNG(SeedSeq& seq);

      RNG& operator=(const pcg32_random_t& other);

      template <typename SeedSeq>
      void seed(SeedSeq& seq);

      void seed(RNG& other);

      result_type operator()();
      result_type operator()(result_type bound);
      // std::seed_seq interface
      template <typename It>
      void generate(It start, It end);

      std::size_t size() const noexcept;

      template <class OutputIt>
      void param(OutputIt dest) const;
    };
  }
}