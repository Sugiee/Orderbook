#include <algorithm>
#include <iostream>
#include <cstdint>
#include <vector>
#include <sstream>
#include <memory>
#include <list>
#include <map>
#include <unordered_map>
#include <numeric>


enum class OrderType {
    GoodTillCancel, // add to order book first, then it will match
    FillAndKill // sent to order book and it will attempt to fill as many positions as possible, and anything that is left is took out the orderbook
};

enum class Side {
    Buy, 
    Sell
};

using Price = std::uint32_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;

struct LevelInfo { // used with public APIs to get state of the orderbook
    Price price_;
    Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;

// encapsulate LevelInfo object to represent out sides. Each side is a list of LevelInfos
class OrderbookLevelInfos {
public:  
    // Constructor
    OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks) 
        : bids_{ bids }
        , asks_{ asks } 
    {}
    
    // Public APIs
    const LevelInfos& GetBids() const { return bids_; }
    const LevelInfos& GetAsks() const { return asks_; }

private:
    // Private APIS
    LevelInfos bids_;
    LevelInfos asks_;
};

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

struct TradeInfo {
    OrderId orderId_;
    Price price_;
    Quantity quantity_;
};

class Trade {
public:
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade) 
        : bidTrade_ { bidTrade }
        , askTrade_ { askTrade }
    {}

    const TradeInfo& GetBidTrade() const { return bidTrade_; }
    const TradeInfo& GetAskTrade() const { return askTrade_; }

private:
    TradeInfo bidTrade_;    
    TradeInfo askTrade_;    
};

using Trades = std::vector<Trade>;

// maps to represent bids and asks in ascending/descending order
class Orderbook {
private:

    struct OrderEntry {
        OrderPointer order_ { nullptr };
        OrderPointers::iterator location_;
    };

    std::map<Price, OrderPointers, std::greater<Price>> bids_; 
    std::map<Price, OrderPointers, std::less<Price>> asks_;
    std::unordered_map<OrderId, OrderEntry> orders_;

    bool canMatch(Side side, Price price) const {
        if (side == Side::Buy) {
            if (asks_.empty()) 
                return false;

            const auto& [bestAsk, _] = *asks_.begin(); // structured binding
            return price >= bestAsk;
        }
        else {
            if(bids_.empty())
                return false;

            const auto& [bestBid, _] = *bids_.begin();
            return price <= bestBid; 
        }
    }

    Trades MatchOrders() {
        Trades trades;
        trades.reserve(orders_.size());
    
        while (true) {
            if (bids_.empty() || asks_.empty())
                break;

            auto& [bidPrice, bids] = *bids_.begin();
            auto& [askPrice, asks] = *asks_.begin();

            if (bidPrice < askPrice) 
                break;
            
            while (bids.size() && asks.size()) {
                auto& bid = bids.front();
                auto& ask = asks.front();

                Quantity quantity = std::min(bid->GetRemainingQuantity(),  ask->GetRemainingQuantity());

                bid->Fill(quantity);
                ask->Fill(quantity);

                if (bid->IsFilled()) {
                    bids.pop_front();
                    orders_.erase(bid->GetOrderId());
                }
                
                if (ask->IsFilled()) {
                    asks.pop_front();
                    orders_.erase(ask->GetOrderId());
                }

                if (bids.empty())
                    bids_.erase(bidPrice);
                
                if (asks.empty())
                    asks_.erase(askPrice);

                trades.push_back(Trade {
                    TradeInfo{ bid->GetOrderId(), bid->GetPrice(), quantity },
                    TradeInfo{ bid->GetOrderId(), bid->GetPrice(), quantity } 
                });  
            }
        }

        if (!bids_.empty()) {
            auto& [_, bids] = *bids_.begin();
            auto& order = bids.front();
            if (order->GetOrderType() == OrderType::FillAndKill)
                CancelOrder(order->GetOrderId());
        }

        if (!asks_.empty()) {
            auto& [_, asks] = *asks_.begin();
            auto& order = asks.front();
            if (order->GetOrderType() == OrderType::FillAndKill)
                CancelOrder(order->GetOrderId());
        }

        return trades;
    }

public: // Public API
    Trades AddOrder(OrderPointer order) {
        if (orders_.find(order->GetOrderId()) != orders_.end())
            return { };

        if (order->GetOrderType() == OrderType::FillAndKill && !canMatch(order->GetSide(), order->GetPrice()))
            return { };

        OrderPointers:: iterator iterator;

        if (order->GetSide() == Side::Buy) {
            auto& orders = bids_[order->GetPrice()];
            orders.push_back(order);
            iterator = std::next(orders.begin(), orders.size() -1);
        } 
        else {
            auto& orders = asks_[order->GetPrice()];
            orders.push_back(order);
            iterator = std::next(orders.begin(), orders.size() - 1);
        }

        orders_.insert({ order->GetOrderId(), OrderEntry{ order, iterator } });
        return MatchOrders();
    }

    void CancelOrder(OrderId orderId) {
        if (!(orders_.find(orderId) != orders_.end()))
            return;

        const auto& [order, iterator] = orders_.at(orderId);
        orders_.erase(orderId);

        if (order->GetSide() == Side::Sell) {
            auto price = order->GetPrice();
            auto& orders = asks_.at(price);
            orders.erase(iterator);
            if (orders.empty())
                asks_.erase(price);
        }
        else {
            auto price = order->GetPrice();
            auto& orders = bids_.at(price);
            orders.erase(iterator);
            if (orders.empty())
                bids_.erase(price);
        }
    }

    Trades MatchOrder(orderModify order) {
        if (!(orders_.find(order.GetOrderId()) != orders_.end()))
            return { };
        
        const auto& [existingOrder, _] = orders_.at(order.GetOrderId());
        CancelOrder(order.GetOrderId());
        return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
    }

    std::size_t Size() const { return orders_.size(); }

    OrderbookLevelInfos GetOrderInfos() const {
        LevelInfos bidInfos, askInfos;
        bidInfos.reserve(orders_.size());
        askInfos.reserve(orders_.size());

        //lambda
        auto CreateLevelInfos = [](Price price, const OrderPointers& orders) {
            return LevelInfo{ price, std::accumulate(orders.begin(), orders.end(), (Quantity)0, 
                [](Quantity runningSum, const OrderPointer& order)
                { return runningSum + order->GetRemainingQuantity(); }) };
        };

        for (const auto& [price, orders] : bids_) 
            bidInfos.push_back(CreateLevelInfos(price, orders));

        for (const auto& [price, orders] : asks_) 
            askInfos.push_back(CreateLevelInfos(price, orders));

        return OrderbookLevelInfos{ bidInfos, askInfos };
        
    }

};


int main() {
    
    Orderbook orderbook;
    const OrderId orderId = 1;
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, orderId, Side::Buy, 100, 10));
    std::cout << orderbook.Size() << std::endl; // 1
    orderbook.CancelOrder(orderId);
    std::cout << orderbook.Size() << std::endl; // 0
    
    return 0;
}

