#pragma once

#include <limits>

#include "Usings.h"

struct Constants { // for market Order, dont care what price order gets filled at
    static const Price InvalidPrice = std::numeric_limits<Price>::quiet_NaN();
};