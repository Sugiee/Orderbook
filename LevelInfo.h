#pragma once

#include "Usings.h"

struct LevelInfo { // used with public APIs to get state of the orderbook
    Price price_;
    Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;