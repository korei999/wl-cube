#pragma once

#include <climits>
#include <random>

#include "ultratypes.h"

namespace rng
{

extern std::mt19937 mt;

static inline int
get(int min, int max)
{
    return std::uniform_int_distribution {min, max}(mt);
}

static inline int
get()
{
    return std::uniform_int_distribution {INT_MIN, INT_MAX}(mt);
}

static inline f32
get(f32 min, f32 max)
{
    return std::uniform_real_distribution {min, max}(mt);
}

} /* namespace rng */
