# Order Book Implementation in C++

This project provides a simple implementation of an order book for a trading system. It supports basic operations like adding orders, canceling orders, and matching orders. The order book maintains a list of buy and sell orders, handles order matching, and provides an interface to query the current state of the order book.

## Features

- **Add Order**: Allows adding orders to the order book. Orders can be of type `GoodTillCancel` or `FillAndKill`.
- **Cancel Order**: Allows canceling an existing order using its `OrderId`.
- **Match Orders**: Automatically matches buy and sell orders based on price and quantity.
- **Order State**: Provides a view of the current state of the order book with details of bid and ask levels.

## Code Structure

- `Orderbook`: Main class representing the order book, containing all the orders and logic for matching and canceling orders.
- `Order`: Represents an individual order with details like order type, price, quantity, etc.
- `OrderType`, `Side`: Enumerations defining types of orders and the side (buy/sell) of the order.
- `OrderbookLevelInfos`, `LevelInfo`: Classes to encapsulate and provide information about different levels in the order book.
- `Trade`: Class to represent a trade resulting from matching orders.
- `orderModify`: Class to handle modifications to existing orders.
