#pragma once

#include <list>
#include <exception>
#include <sstream>
#include <memory>

#include "OrderType.h"
#include "Side.h"
#include "Usings.h"
#include "Constants.h"

class Order {
public:
    Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity) 
        : orderType_ { orderType }
        , orderId_ { orderId }
        , side_ { side }
        , price_ { price }
        , initialQuantity_ { quantity }
        , remainingQuantity_ { quantity }
    {}

    // Public APIs
    OrderType GetOrderType() const { return orderType_; }
    OrderId GetOrderId() const { return orderId_; }
    Side GetSide() const { return side_; }
    Price GetPrice() const { return price_; }
    Quantity GetInitialQuantity() const { return initialQuantity_; }
    Quantity GetRemainingQuantity() const { return remainingQuantity_; }
    Quantity GetFilledQuantity() const { return initialQuantity_ - remainingQuantity_; }
    bool IsFilled() const { return GetRemainingQuantity() == 0; }
    void Fill(Quantity quantity) {
        if (quantity > GetRemainingQuantity()) {
            std::ostringstream oss;
            oss << "Order ( " << GetOrderId() << ") cannot be filled for more than its remaining quantity.";
            throw std::logic_error(oss.str());
        }

        remainingQuantity_ -= quantity;
    }

private:
    OrderType orderType_;
    OrderId orderId_;
    Side side_;
    Price price_; 
    Quantity initialQuantity_;
    Quantity remainingQuantity_;
};

//reference semantics
using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;