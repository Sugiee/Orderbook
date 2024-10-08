#pragma once

#include "Order.h"

// create an abstraction to modify order
class orderModify {
public:
    orderModify(OrderId orderId, Side side, Price price, Quantity quantity) 
        : orderId_ { orderId }
        , side_ { side }
        , price_ { price }
        , quantity_ { quantity }
    {}

    OrderId GetOrderId() const { return orderId_; }
    Side GetSide() const { return side_; }
    Price GetPrice() const { return price_; }
    Quantity GetQuantity() const { return quantity_; }

    OrderPointer ToOrderPointer(OrderType type) const {
        return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());
    }

private:
    OrderId orderId_;
    Side side_;
    Price price_; 
    Quantity quantity_;
};