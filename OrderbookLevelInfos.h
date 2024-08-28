#pragma once

#include "LevelInfo.h"

// encapsulate LevelInfo object to represent out sides. Each side is a list of LevelInfos
class OrderbookLevelInfos {
public:  
    OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks) 
        : bids_{ bids }
        , asks_{ asks } 
    {}
    
    // Public APIs
    const LevelInfos& GetBids() const { return bids_; }
    const LevelInfos& GetAsks() const { return asks_; }

private:
    // Private APIs
    LevelInfos bids_;
    LevelInfos asks_;
};