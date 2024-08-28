#pragma once

enum class OrderType {
    GoodTillCancel, // Add to order book first, then it will match
    FillAndKill     // Sent to order book and it will attempt to fill as many positions as possible, 
};                  // and anything that is left is took out the orderbook