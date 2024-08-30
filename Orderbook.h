#pragma once

#include <map>
#include <unordered_map>
#include <thread>
#include <condition_variable>
#include <mutex>

#include "Usings.h"
#include "Order.h"
#include "OrderModify.h"
#include "OrderbookLevelInfos.h"
#include "Trade.h"

class Orderbook {
public:
    Trades AddOrder(OrderPointer order);
    void CancelOrder(OrderId orderId);
    Trades ModifyOrder(orderModify order);
    
    OrderbookLevelInfos GetOrderInfos() const;
    std::size_t Size() const;

private:
    struct OrderEntry {
        OrderPointer order_ { nullptr };
        OrderPointers::iterator location_;
    };

    // maps to represent bids and asks in ascending/descending order
    std::map<Price, OrderPointers, std::greater<Price>> bids_; 
    std::map<Price, OrderPointers, std::less<Price>> asks_;
    std::unordered_map<OrderId, OrderEntry> orders_;

    mutable std::mutex ordersMutex_;

    // internal functions
    bool CanMatch(Side side, Price price) const;
    Trades MatchOrders();
};