#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <queue>
#include <iomanip>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <cmath>
#include <numeric>
#include <optional>
#include <ctime>

using namespace std;

enum OrderType { LIMIT, MARKET, STOP, IOC, FOK };
enum OrderStatus { OPEN, PARTIAL, FILLED, CANCELLED, REJECTED };

struct Order {
    int id;
    double price;
    double quantity;
    double filledQty;
    string side;
    OrderType type;
    OrderStatus status;
    chrono::system_clock::time_point timestamp;
    string clientId;
    chrono::nanoseconds latency;
    double stopPrice;
};

struct Trade {
    int buyOrderId;
    int sellOrderId;
    double price;
    double quantity;
    chrono::system_clock::time_point timestamp;
    double fee;
};

struct Position {
    double quantity = 0.0;
    double avgPrice = 0.0;
};

class OrderBook {
private:
    map<double, vector<Order>, greater<double>> bids;
    map<double, vector<Order>, less<double>> asks;
    unordered_map<int, Order> orderTracker;
    unordered_map<string, Position> clientPositions;
    vector<Trade> tradeHistory;
    int orderCounter = 0;
    optional<double> bestBid;
    optional<double> bestAsk;
    double makerFee = 0.001;
    double takerFee = 0.002;
    double minPrice = 0.01;
    double minQty = 0.00001;
    const double EPSILON = 1e-6;
    const chrono::nanoseconds SIMULATED_LATENCY = chrono::nanoseconds(5000);

    vector<function<void(const Trade&)>> tradeListeners;
    vector<function<void(const Order&)>> orderListeners;

    void updateMarketData() {
        bestBid = bids.empty() ? optional<double>{} : bids.begin()->first;
        bestAsk = asks.empty() ? optional<double>{} : asks.begin()->first;
    }

    void updateOrderStatus(int orderId, OrderStatus status, double filledQty = 0) {
        if (orderTracker.count(orderId)) {
            orderTracker[orderId].status = status;
            orderTracker[orderId].filledQty += filledQty;
            notifyOrderListeners(orderTracker[orderId]);
        }
    }

    void updatePosition(const Trade& trade) {
        string buyClient = orderTracker[trade.buyOrderId].clientId;
        string sellClient = orderTracker[trade.sellOrderId].clientId;

        if (!buyClient.empty()) {
            Position& pos = clientPositions[buyClient];
            double totalCost = pos.quantity * pos.avgPrice + trade.quantity * trade.price;
            pos.quantity += trade.quantity;
            pos.avgPrice = pos.quantity ? totalCost / pos.quantity : 0.0;
        }

        if (!sellClient.empty()) {
            Position& pos = clientPositions[sellClient];
            double totalCost = pos.quantity * pos.avgPrice - trade.quantity * trade.price;
            pos.quantity -= trade.quantity;
            pos.avgPrice = pos.quantity ? totalCost / pos.quantity : 0.0;
        }
    }

    bool validateOrder(double price, double quantity, OrderType type, double stopPrice = 0.0) {
        cout << "Validating: price=" << price << ", qty=" << quantity << ", type=" << type << endl;
        if (quantity <= 0) {
            cout << "Fail: quantity <= 0" << endl;
            return false;
        }
        if (quantity < minQty) {
            cout << "Fail: quantity < minQty (" << minQty << ")" << endl;
            return false;
        }
        double qtyRatio = quantity / minQty;
        if (fabs(qtyRatio - round(qtyRatio)) > EPSILON) {
            cout << "Fail: quantity not a multiple of minQty (ratio=" << qtyRatio << ")" << endl;
            return false;
        }

        if (type != MARKET && type != STOP) {
            if (price <= 0) {
                cout << "Fail: price <= 0" << endl;
                return false;
            }
            if (price < minPrice) {
                cout << "Fail: price < minPrice (" << minPrice << ")" << endl;
                return false;
            }
            double priceRatio = price / minPrice;
            if (fabs(priceRatio - round(priceRatio)) > EPSILON) {
                cout << "Fail: price not a multiple of minPrice (ratio=" << priceRatio << ")" << endl;
                return false;
            }
        }

        if (type == STOP && (stopPrice <= 0 || stopPrice < minPrice)) {
            cout << "Fail: stopPrice <= 0 or < minPrice (" << minPrice << ")" << endl;
            return false;
        }
        cout << "Validation passed" << endl;
        return true;
    }

    double getAvailableQty(const string& side) {
        if (side == "buy") {
            if (asks.empty()) return 0.0;
            double total = 0.0;
            for (const auto& [price, orders] : asks) {
                for (const auto& order : orders) total += order.quantity - order.filledQty;
            }
            return total;
        } else {
            if (bids.empty()) return 0.0;
            double total = 0.0;
            for (const auto& [price, orders] : bids) {
                for (const auto& order : orders) total += order.quantity - order.filledQty;
            }
            return total;
        }
    }

    void notifyTradeListeners(const Trade& trade) {
        for (const auto& listener : tradeListeners) listener(trade);
    }

    void notifyOrderListeners(const Order& order) {
        for (const auto& listener : orderListeners) listener(order);
    }

    void checkStopOrders(double lastPrice) {
        for (auto& [id, order] : orderTracker) {
            if (order.type == STOP && order.status == OPEN) {
                bool trigger = (order.side == "buy" && lastPrice >= order.stopPrice) ||
                              (order.side == "sell" && lastPrice <= order.stopPrice);
                if (trigger) {
                    cout << "✅ Stop Order Triggered [ID:" << id << "]: " << order.side << " " << fixed << setprecision(6) << order.quantity 
                         << " @ " << order.price << " (Triggered at: " << lastPrice << ")" << endl;
                    order.type = LIMIT;
                    if (order.side == "buy") bids[order.price].push_back(order);
                    else asks[order.price].push_back(order);
                    matchOrders();
                }
            }
        }
    }

    string formatTotal(double total) {
        if (total >= 1e6) {
            return to_string(total / 1e6) + "M";
        } else if (total >= 1e3) {
            return to_string(total / 1e3) + "K";
        }
        return to_string(total);
    }

    double roundFee(double fee) {
        return round(fee * 100) / 100;
    }

    string formatTimestamp(const chrono::system_clock::time_point& tp) {
        auto time = chrono::system_clock::to_time_t(tp);
        stringstream ss;
        ss << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

public:
    int placeOrder(string side, double price, double quantity, OrderType type, string clientId = "", double stopPrice = 0.0) {
        auto submitTime = chrono::system_clock::now();
        if (!validateOrder(price, quantity, type, stopPrice)) {
            cout << "❌ Invalid Order: Price/Quantity must be positive and meet tick size" << endl;
            return -1;
        }

        orderCounter++;
        Order order = {orderCounter, price, quantity, 0, side, type, OPEN, submitTime, clientId,
                      SIMULATED_LATENCY, stopPrice};
        orderTracker[orderCounter] = order;

        cout << fixed << setprecision(2);
        if (type == MARKET) {
            cout << "✅ Order Placed [ID:" << orderCounter << "]: " << side << " " << fixed << setprecision(6) << quantity 
                 << " @ N/A (MARKET) Client: " << clientId << " Latency: " << order.latency.count() << "ns" << endl;
        } else {
            cout << "✅ Order Placed [ID:" << orderCounter << "]: " << side << " " << fixed << setprecision(6) << quantity 
                 << " @ " << price << " (" << (type == LIMIT ? "LIMIT" : type == IOC ? "IOC" : "FOK") 
                 << ") Client: " << clientId << " Latency: " << order.latency.count() << "ns" << endl;
        }

        if (type == MARKET) {
            double availableQty = getAvailableQty(side);
            if (availableQty < quantity) {
                cout << "⚠️ Warning: Insufficient liquidity for market order. Available: " << fixed << setprecision(6) << availableQty 
                     << ", Requested: " << quantity << endl;
            }
            double avgPrice = placeMarketOrder(side, quantity, clientId);
            if (avgPrice > 0) {
                cout << "Market Order Executed [ID:" << orderCounter << "]: Avg Price: " << fixed << setprecision(2) << avgPrice << endl;
            }
            return orderCounter;
        } else if (type == STOP) {
            cout << "✅ Stop Order Placed [ID:" << orderCounter << "]: " << side << " " << fixed << setprecision(6) << quantity 
                 << " @ " << price << " (Stop: " << stopPrice << ") Client: " << clientId << endl;
            return orderCounter;
        }

        if (side == "buy") bids[price].push_back(order);
        else asks[price].push_back(order);

        if (type == LIMIT) matchOrders();
        else if (type == IOC) {
            matchOrders();
            if (orderTracker[orderCounter].status == OPEN) cancelOrder(orderCounter);
        } else if (type == FOK) {
            double availableQty = getAvailableQty(side);
            if (availableQty >= quantity) matchOrders();
            else {
                cancelOrder(orderCounter);
                updateOrderStatus(orderCounter, REJECTED);
                cout << "❌ FOK Order Rejected: Insufficient liquidity" << endl;
            }
        }

        updateMarketData();
        return orderCounter;
    }

    bool modifyOrder(int orderId, double newPrice, double newQuantity) {
        if (!orderTracker.count(orderId) || orderTracker[orderId].status != OPEN) {
            cout << "❌ Cannot modify order ID " << orderId << ": Not found or not open" << endl;
            return false;
        }
        if (!validateOrder(newPrice, newQuantity, LIMIT)) {
            cout << "❌ Invalid modification: Price/Quantity invalid" << endl;
            return false;
        }

        Order oldOrder = orderTracker[orderId];
        cancelOrder(orderId);
        int newId = placeOrder(oldOrder.side, newPrice, newQuantity, LIMIT, oldOrder.clientId);
        cout << "✅ Order Modified: ID " << orderId << " -> New ID " << newId << endl;
        return true;
    }

    double placeMarketOrder(string side, double quantity, string clientId) {
        double remainingQty = quantity;
        double totalCost = 0.0;
        double totalFilled = 0.0;

        if (side == "buy") {
            remainingQty = executeMarketOrder(asks, quantity, "buy", clientId, true, totalCost, totalFilled);
        } else {
            remainingQty = executeMarketOrder(bids, quantity, "sell", clientId, true, totalCost, totalFilled);
        }

        updateOrderStatus(orderCounter, remainingQty == quantity ? REJECTED : 
                         (remainingQty > 0 ? PARTIAL : FILLED), quantity - remainingQty);
        if (remainingQty > 0) cout << "⚠️ Partial Fill: Remaining Qty " << fixed << setprecision(6) << remainingQty << endl;
        updateMarketData();

        if (!tradeHistory.empty()) checkStopOrders(tradeHistory.back().price);

        if (totalFilled > 0) {
            return totalCost / totalFilled;
        }
        return 0.0;
    }

    bool cancelOrder(int orderId) {
        bool removed = removeOrder(bids, orderId) || removeOrder(asks, orderId);
        if (removed) {
            updateOrderStatus(orderId, CANCELLED);
            cout << "✅ Order ID " << orderId << " cancelled" << endl;
            updateMarketData();
            return true;
        }
        cout << "❌ Order ID " << orderId << " not found" << endl;
        return false;
    }

    void matchOrders() {
        while (!bids.empty() && !asks.empty() && bids.begin()->first >= asks.begin()->first) {
            auto& bidOrders = bids.begin()->second;
            auto& askOrders = asks.begin()->second;

            while (!bidOrders.empty() && !askOrders.empty()) {
                Order& buyOrder = bidOrders.front();
                Order& sellOrder = askOrders.front();
                double tradeQty = min(buyOrder.quantity - buyOrder.filledQty, 
                                    sellOrder.quantity - sellOrder.filledQty);
                double tradePrice = sellOrder.timestamp < buyOrder.timestamp ? 
                                  sellOrder.price : buyOrder.price;

                Trade trade = {buyOrder.id, sellOrder.id, tradePrice, tradeQty, 
                              chrono::system_clock::now(), makerFee * tradeQty * tradePrice};
                tradeHistory.push_back(trade);
                updatePosition(trade);
                notifyTradeListeners(trade);

                cout << "💰 MARKET TRADE: " << fixed << setprecision(6) << tradeQty << " @ " << tradePrice 
                     << " (Fee: " << fixed << setprecision(2) << roundFee(trade.fee) << ")" << endl;

                buyOrder.filledQty += tradeQty;
                sellOrder.filledQty += tradeQty;

                updateOrderStatus(buyOrder.id, buyOrder.filledQty == buyOrder.quantity ? 
                                FILLED : PARTIAL, tradeQty);
                updateOrderStatus(sellOrder.id, sellOrder.filledQty == sellOrder.quantity ? 
                                FILLED : PARTIAL, tradeQty);

                if (buyOrder.filledQty >= buyOrder.quantity) bidOrders.erase(bidOrders.begin());
                if (sellOrder.filledQty >= sellOrder.quantity) askOrders.erase(askOrders.begin());
            }

            if (bidOrders.empty()) bids.erase(bids.begin());
            if (askOrders.empty()) asks.erase(asks.begin());
        }
        updateMarketData();
        if (!tradeHistory.empty()) checkStopOrders(tradeHistory.back().price);
    }

    template <typename Compare>
    bool removeOrder(map<double, vector<Order>, Compare>& book, int orderId) {
        for (auto it = book.begin(); it != book.end(); ++it) {
            auto& orders = it->second;
            auto newEnd = remove_if(orders.begin(), orders.end(),
                                  [&](Order& o) { return o.id == orderId; });
            if (newEnd != orders.end()) {
                orders.erase(newEnd, orders.end());
                if (orders.empty()) book.erase(it);
                return true;
            }
        }
        return false;
    }

    template <typename Compare>
    double executeMarketOrder(map<double, vector<Order>, Compare>& book, double quantity, 
                             string side, string clientId, bool isTaker, double& totalCost, double& totalFilled) {
        double remainingQty = quantity;
        auto it = book.begin();

        while (it != book.end() && remainingQty > 0) {
            auto& orders = it->second;
            while (!orders.empty() && remainingQty > 0) {
                Order& order = orders.front();
                double tradeQty = min(remainingQty, order.quantity - order.filledQty);
                double fee = isTaker ? takerFee * tradeQty * it->first : makerFee * tradeQty * it->first;

                Trade trade = {side == "buy" ? orderCounter : order.id,
                              side == "buy" ? order.id : orderCounter,
                              it->first, tradeQty, chrono::system_clock::now(), 
                              fee};
                tradeHistory.push_back(trade);
                updatePosition(trade);
                notifyTradeListeners(trade);

                cout << "💰 MARKET TRADE: " << fixed << setprecision(6) << tradeQty << " @ " << it->first 
                     << " (Fee: " << fixed << setprecision(2) << roundFee(fee) << ")" << endl;

                totalCost += tradeQty * it->first;
                totalFilled += tradeQty;

                order.filledQty += tradeQty;
                remainingQty -= tradeQty;

                updateOrderStatus(order.id, order.filledQty == order.quantity ? 
                                FILLED : PARTIAL, tradeQty);
                if (order.filledQty >= order.quantity) orders.erase(orders.begin());
            }
            if (orders.empty()) {
                auto toErase = it;
                ++it;
                book.erase(toErase);
            } else {
                ++it;
            }
        }
        return remainingQty;
    }

    void printOrderBook(int depth = 5) {
        cout << fixed << setprecision(2);
        cout << "\n===== ORDER BOOK =====\n";
        cout << "Spread: " << getSpread() << " | Mid: " << getMidPrice() << endl;

        cout << "Price(USDT)\tAmount(BTC)\tTotal(USDT)\n";
        cout << "ASKS (Sell) [Red in UI]\n";
        double askCumulativeTotal = 0.0;
        int askCount = 0;
        for (auto it = asks.begin(); it != asks.end() && askCount < depth; ++it) {
            double totalQty = 0;
            for (const auto& order : it->second) totalQty += order.quantity - order.filledQty;
            askCumulativeTotal += it->first * totalQty;
            cout << it->first << "\t\t" << fixed << setprecision(6) << totalQty 
                 << "\t\t" << formatTotal(askCumulativeTotal) << endl;
            askCount++;
        }

        cout << "---------------------\n";

        cout << "BIDS (Buy) [Green in UI]\n";
        double bidCumulativeTotal = 0.0;
        int bidCount = 0;
        for (auto it = bids.begin(); it != bids.end() && bidCount < depth; ++it) {
            double totalQty = 0;
            for (const auto& order : it->second) totalQty += order.quantity - order.filledQty;
            bidCumulativeTotal += it->first * totalQty;
            cout << it->first << "\t\t" << fixed << setprecision(6) << totalQty 
                 << "\t\t" << formatTotal(bidCumulativeTotal) << endl;
            bidCount++;
        }
        cout << "=====================\n";
    }

    void printDepthChart() {
        cout << fixed << setprecision(2);
        cout << "\n===== DEPTH CHART SIMULATION =====\n";

        cout << "ASKS (Sell Orders Volume) [Red in UI]\n";
        double askCumulativeVolume = 0.0;
        for (auto it = asks.begin(); it != asks.end(); ++it) {
            double totalQty = 0;
            for (const auto& order : it->second) totalQty += order.quantity - order.filledQty;
            askCumulativeVolume += totalQty;
            cout << "Price: " << it->first << " | Volume: " << fixed << setprecision(6) << askCumulativeVolume << endl;
        }

        cout << "---------------------\n";

        cout << "BIDS (Buy Orders Volume) [Green in UI]\n";
        double bidCumulativeVolume = 0.0;
        for (auto it = bids.begin(); it != bids.end(); ++it) {
            double totalQty = 0;
            for (const auto& order : it->second) totalQty += order.quantity - order.filledQty;
            bidCumulativeVolume += totalQty;
            cout << "Price: " << it->first << " | Volume: " << fixed << setprecision(6) << bidCumulativeVolume << endl;
        }
        cout << "=====================\n";
    }

    void detectSupportResistance(double threshold = 1.0) {
        cout << "\n===== SUPPORT/RESISTANCE LEVELS =====\n";
        for (auto it = bids.begin(); it != bids.end(); ++it) {
            double totalQty = 0;
            for (const auto& order : it->second) totalQty += order.quantity - order.filledQty;
            if (totalQty >= threshold) {
                cout << "Support (Buy Wall) at Price: " << it->first << " | Qty: " << fixed << setprecision(6) << totalQty << endl;
            }
        }
        for (auto it = asks.begin(); it != asks.end(); ++it) {
            double totalQty = 0;
            for (const auto& order : it->second) totalQty += order.quantity - order.filledQty;
            if (totalQty >= threshold) {
                cout << "Resistance (Sell Wall) at Price: " << it->first << " | Qty: " << fixed << setprecision(6) << totalQty << endl;
            }
        }
        cout << "=====================\n";
    }

    void printMatchedTrades() {
        cout << "\n===== MATCHED TRADES =====\n";
        for (const auto& trade : tradeHistory) {
            cout << "Timestamp: " << formatTimestamp(trade.timestamp) 
                 << " | Price: " << trade.price 
                 << " | Qty: " << fixed << setprecision(6) << trade.quantity 
                 << " | BuyID: " << trade.buyOrderId 
                 << " | SellID: " << trade.sellOrderId 
                 << " | Fee: " << fixed << setprecision(2) << roundFee(trade.fee) << endl;
        }
        cout << "========================\n";
    }

    void printOrderStatus(int orderId) {
        if (!orderTracker.count(orderId)) {
            cout << "❌ Order ID " << orderId << " not found" << endl;
            return;
        }
        const Order& order = orderTracker[orderId];
        cout << "\n===== ORDER STATUS =====\n";
        cout << "ID: " << order.id << " | Side: " << order.side 
             << " | Price: " << order.price << " | Qty: " << fixed << setprecision(6) << order.quantity 
             << " | Filled: " << fixed << setprecision(6) << order.filledQty << " | Latency: " << order.latency.count() << "ns";
        if (order.type == STOP) cout << " | Stop Price: " << order.stopPrice;
        cout << endl;
        cout << "Status: " << (order.status == OPEN ? "OPEN" : 
                             order.status == PARTIAL ? "PARTIAL" : 
                             order.status == FILLED ? "FILLED" : 
                             order.status == CANCELLED ? "CANCELLED" : "REJECTED") << endl;
        cout << "=====================\n";
    }

    void printClientPosition(string clientId) {
        if (!clientPositions.count(clientId)) {
            cout << "❌ Client " << clientId << " has no position" << endl;
            return;
        }
        const Position& pos = clientPositions[clientId];
        cout << "\n===== POSITION =====\n";
        cout << "Client: " << clientId << " | Qty: " << fixed << setprecision(6) << pos.quantity 
             << " | Avg Price: " << pos.avgPrice << endl;
        cout << "=====================\n";
    }

    void saveSnapshot(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cout << "❌ Failed to save snapshot" << endl;
            return;
        }

        file << "BIDS\n";
        for (const auto& [price, orders] : bids) {
            for (const auto& order : orders) {
                file << order.id << "," << order.side << "," << fixed << setprecision(6) << order.price << "," 
                     << order.quantity << "," << order.filledQty << "," << order.type << ","
                     << order.status << "," << order.clientId << "," << order.stopPrice << "\n";
            }
        }

        file << "ASKS\n";
        for (const auto& [price, orders] : asks) {
            for (const auto& order : orders) {
                file << order.id << "," << order.side << "," << fixed << setprecision(6) << order.price << "," 
                     << order.quantity << "," << order.filledQty << "," << order.type << ","
                     << order.status << "," << order.clientId << "," << order.stopPrice << "\n";
            }
        }

        file.close();
        cout << "✅ Snapshot saved to " << filename << endl;
    }

    void loadSnapshot(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "❌ Failed to load snapshot" << endl;
            return;
        }

        bids.clear();
        asks.clear();
        orderTracker.clear();

        string line, section;
        while (getline(file, line)) {
            if (line == "BIDS" || line == "ASKS") {
                section = line;
                continue;
            }
            if (line.empty()) continue;

            Order order;
            stringstream ss(line);
            string typeStr, statusStr;
            getline(ss, typeStr, ','); order.id = stoi(typeStr);
            getline(ss, order.side, ',');
            getline(ss, typeStr, ','); order.price = stod(typeStr);
            getline(ss, typeStr, ','); order.quantity = stod(typeStr);
            getline(ss, typeStr, ','); order.filledQty = stod(typeStr);
            getline(ss, typeStr, ','); order.type = static_cast<OrderType>(stoi(typeStr));
            getline(ss, statusStr, ','); order.status = static_cast<OrderStatus>(stoi(statusStr));
            getline(ss, order.clientId, ',');
            getline(ss, typeStr, ','); order.stopPrice = stod(typeStr);
            order.timestamp = chrono::system_clock::now();

            orderTracker[order.id] = order;
            if (section == "BIDS" && order.type != STOP) bids[order.price].push_back(order);
            else if (section == "ASKS" && order.type != STOP) asks[order.price].push_back(order);
        }

        orderCounter = 0;
        for (const auto& [id, _] : orderTracker) {
            orderCounter = max(orderCounter, id);
        }
        updateMarketData();
        file.close();
        cout << "✅ Snapshot loaded from " << filename << endl;
    }

    optional<double> getBestBid() const { return bestBid; }
    optional<double> getBestAsk() const { return bestAsk; }
    double getMidPrice() const { 
        if (bestBid && bestAsk) return (*bestBid + *bestAsk) / 2.0; 
        return bestBid ? *bestBid : bestAsk ? *bestAsk : 0.0;
    }
    double getSpread() const { 
        if (bestBid && bestAsk) return *bestAsk - *bestBid; 
        return 0.0;
    }
    void setFees(double maker, double taker) { makerFee = maker; takerFee = taker; }
    void setTickSize(double price, double qty) { minPrice = price; minQty = qty; }

    void onTrade(function<void(const Trade&)> callback) { tradeListeners.push_back(callback); }
    void onOrder(function<void(const Order&)> callback) { orderListeners.push_back(callback); }
};

int main() {
    OrderBook ob;
    ob.setFees(0.001, 0.002);
    ob.setTickSize(0.01, 0.00001);

    ob.onTrade([](const Trade& t) {
        cout << "📡 TRADE EVENT: " << t.quantity << " @ " << t.price << endl;
    });
    ob.onOrder([](const Order& o) {
        cout << "📡 ORDER EVENT: ID " << o.id << " Status: " 
             << (o.status == OPEN ? "OPEN" : o.status == PARTIAL ? "PARTIAL" : "FILLED") << endl;
    });

    int id1 = ob.placeOrder("buy", 67416.03, 1.04760, LIMIT, "Client1");
    int id2 = ob.placeOrder("buy", 67416.01, 0.00018, LIMIT, "Client2");
    int id3 = ob.placeOrder("buy", 67416.00, 0.04563, LIMIT, "Client3");
    int id4 = ob.placeOrder("buy", 67415.99, 0.29541, LIMIT, "Client4");
    int id5 = ob.placeOrder("buy", 67415.98, 0.00010, LIMIT, "Client5");
    int id6 = ob.placeOrder("sell", 67416.04, 2.56276, LIMIT, "Client6");
    int id7 = ob.placeOrder("sell", 67416.06, 0.00018, LIMIT, "Client7");
    int id8 = ob.placeOrder("sell", 67416.07, 0.23883, LIMIT, "Client8");
    int id9 = ob.placeOrder("sell", 67416.08, 0.00010, LIMIT, "Client9");
    int id10 = ob.placeOrder("sell", 67416.11, 0.00010, LIMIT, "Client10");
    int id11 = ob.placeOrder("sell", 67416.12, 0.50000, LIMIT, "Client12");
    int id12 = ob.placeOrder("sell", 67416.13, 0.50000, LIMIT, "Client13");
    int id13 = ob.placeOrder("buy", 67415.97, 5.00000, LIMIT, "Client14");
    int id14 = ob.placeOrder("sell", 67416.14, 1.00000, LIMIT, "Client15");
    int id15 = ob.placeOrder("sell", 67416.15, 1.00000, LIMIT, "Client16");
    int id16 = ob.placeOrder("buy", -67416.16, 1.00000, LIMIT, "Client17");

    ob.printOrderBook();
    ob.printDepthChart();
    ob.detectSupportResistance(1.0);

    // Modify an existing buy order (e.g., ID 1) to change its price and quantity
    cout << "\nModifying Order ID 1...\n";
    ob.modifyOrder(id1, 67416.02, 2.00000);  // Modify ID 1 to a new price and quantity

    // Print the order book after modification to see the change
    ob.printOrderBook();
    ob.printDepthChart();
    ob.detectSupportResistance(1.0);

    ob.cancelOrder(id2);

    ob.placeOrder("buy", 0.0, 5.0, MARKET, "Client11");

    ob.printOrderBook();
    ob.printDepthChart();
    ob.detectSupportResistance(1.0);
    ob.printMatchedTrades();
    ob.printOrderStatus(id1);
    ob.printClientPosition("Client1");
    ob.printClientPosition("Client6");

    cout << "Best Bid: " << (ob.getBestBid() ? *ob.getBestBid() : 0.0) 
         << " | Best Ask: " << (ob.getBestAsk() ? *ob.getBestAsk() : 0.0) << endl;

    ob.saveSnapshot("orderbook.snapshot");
    ob.loadSnapshot("orderbook.snapshot");

    ob.printOrderBook();
    return 0;
}


/*

Validating: price=67416, qty=1.0476, type=0
Validation passed
✅ Order Placed [ID:1]: buy 1.047600 @ 67416.030000 (LIMIT) Client: Client1 Latency: 5000ns
Validating: price=67416.010000, qty=0.000180, type=0
Validation passed
✅ Order Placed [ID:2]: buy 0.000180 @ 67416.010000 (LIMIT) Client: Client2 Latency: 5000ns
Validating: price=67416.000000, qty=0.045630, type=0
Validation passed
✅ Order Placed [ID:3]: buy 0.045630 @ 67416.000000 (LIMIT) Client: Client3 Latency: 5000ns
Validating: price=67415.990000, qty=0.295410, type=0
Validation passed
✅ Order Placed [ID:4]: buy 0.295410 @ 67415.990000 (LIMIT) Client: Client4 Latency: 5000ns
Validating: price=67415.980000, qty=0.000100, type=0
Validation passed
✅ Order Placed [ID:5]: buy 0.000100 @ 67415.980000 (LIMIT) Client: Client5 Latency: 5000ns
Validating: price=67416.040000, qty=2.562760, type=0
Validation passed
✅ Order Placed [ID:6]: sell 2.562760 @ 67416.040000 (LIMIT) Client: Client6 Latency: 5000ns
Validating: price=67416.060000, qty=0.000180, type=0
Validation passed
✅ Order Placed [ID:7]: sell 0.000180 @ 67416.060000 (LIMIT) Client: Client7 Latency: 5000ns
Validating: price=67416.070000, qty=0.238830, type=0
Validation passed
✅ Order Placed [ID:8]: sell 0.238830 @ 67416.070000 (LIMIT) Client: Client8 Latency: 5000ns
Validating: price=67416.080000, qty=0.000100, type=0
Validation passed
✅ Order Placed [ID:9]: sell 0.000100 @ 67416.080000 (LIMIT) Client: Client9 Latency: 5000ns
Validating: price=67416.110000, qty=0.000100, type=0
Validation passed
✅ Order Placed [ID:10]: sell 0.000100 @ 67416.110000 (LIMIT) Client: Client10 Latency: 5000ns
Validating: price=67416.120000, qty=0.500000, type=0
Validation passed
✅ Order Placed [ID:11]: sell 0.500000 @ 67416.120000 (LIMIT) Client: Client12 Latency: 5000ns
Validating: price=67416.130000, qty=0.500000, type=0
Validation passed
✅ Order Placed [ID:12]: sell 0.500000 @ 67416.130000 (LIMIT) Client: Client13 Latency: 5000ns
Validating: price=67415.970000, qty=5.000000, type=0
Validation passed
✅ Order Placed [ID:13]: buy 5.000000 @ 67415.970000 (LIMIT) Client: Client14 Latency: 5000ns
Validating: price=67416.140000, qty=1.000000, type=0
Validation passed
✅ Order Placed [ID:14]: sell 1.000000 @ 67416.140000 (LIMIT) Client: Client15 Latency: 5000ns
Validating: price=67416.150000, qty=1.000000, type=0
Validation passed
✅ Order Placed [ID:15]: sell 1.000000 @ 67416.150000 (LIMIT) Client: Client16 Latency: 5000ns
Validating: price=-67416.160000, qty=1.000000, type=0
Fail: price <= 0
❌ Invalid Order: Price/Quantity must be positive and meet tick size

===== ORDER BOOK =====
Spread: 0.01 | Mid: 67416.04
Price(USDT)	Amount(BTC)	Total(USDT)
ASKS (Sell) [Red in UI]
67416.04		2.562760		172.771131K
67416.060000		0.000180		172.783266K
67416.070000		0.238830		188.884246K
67416.080000		0.000100		188.890987K
67416.110000		0.000100		188.897729K
---------------------
BIDS (Buy) [Green in UI]
67416.030000		1.047600		70.625033K
67416.010000		0.000180		70.637168K
67416.000000		0.045630		73.713360K
67415.990000		0.295410		93.628718K
67415.980000		0.000100		93.635459K
=====================

===== DEPTH CHART SIMULATION =====
ASKS (Sell Orders Volume) [Red in UI]
Price: 67416.04 | Volume: 2.562760
Price: 67416.060000 | Volume: 2.562940
Price: 67416.070000 | Volume: 2.801770
Price: 67416.080000 | Volume: 2.801870
Price: 67416.110000 | Volume: 2.801970
Price: 67416.120000 | Volume: 3.301970
Price: 67416.130000 | Volume: 3.801970
Price: 67416.140000 | Volume: 4.801970
Price: 67416.150000 | Volume: 5.801970
---------------------
BIDS (Buy Orders Volume) [Green in UI]
Price: 67416.030000 | Volume: 1.047600
Price: 67416.010000 | Volume: 1.047780
Price: 67416.000000 | Volume: 1.093410
Price: 67415.990000 | Volume: 1.388820
Price: 67415.980000 | Volume: 1.388920
Price: 67415.970000 | Volume: 6.388920
=====================

===== SUPPORT/RESISTANCE LEVELS =====
Support (Buy Wall) at Price: 67416.030000 | Qty: 1.047600
Support (Buy Wall) at Price: 67415.970000 | Qty: 5.000000
Resistance (Sell Wall) at Price: 67416.040000 | Qty: 2.562760
Resistance (Sell Wall) at Price: 67416.140000 | Qty: 1.000000
Resistance (Sell Wall) at Price: 67416.150000 | Qty: 1.000000
=====================

Modifying Order ID 1...
Validating: price=67416.020000, qty=2.000000, type=0
Validation passed
📡 ORDER EVENT: ID 1 Status: FILLED
✅ Order ID 1 cancelled
Validating: price=67416.020000, qty=2.000000, type=0
Validation passed
✅ Order Placed [ID:16]: buy 2.000000 @ 67416.020000 (LIMIT) Client: Client1 Latency: 5000ns
✅ Order Modified: ID 1 -> New ID 16

===== ORDER BOOK =====
Spread: 0.02 | Mid: 67416.03
Price(USDT)	Amount(BTC)	Total(USDT)
ASKS (Sell) [Red in UI]
67416.04		2.562760		172.771131K
67416.060000		0.000180		172.783266K
67416.070000		0.238830		188.884246K
67416.080000		0.000100		188.890987K
67416.110000		0.000100		188.897729K
---------------------
BIDS (Buy) [Green in UI]
67416.020000		2.000000		134.832040K
67416.010000		0.000180		134.844175K
67416.000000		0.045630		137.920367K
67415.990000		0.295410		157.835725K
67415.980000		0.000100		157.842466K
=====================

===== DEPTH CHART SIMULATION =====
ASKS (Sell Orders Volume) [Red in UI]
Price: 67416.04 | Volume: 2.562760
Price: 67416.060000 | Volume: 2.562940
Price: 67416.070000 | Volume: 2.801770
Price: 67416.080000 | Volume: 2.801870
Price: 67416.110000 | Volume: 2.801970
Price: 67416.120000 | Volume: 3.301970
Price: 67416.130000 | Volume: 3.801970
Price: 67416.140000 | Volume: 4.801970
Price: 67416.150000 | Volume: 5.801970
---------------------
BIDS (Buy Orders Volume) [Green in UI]
Price: 67416.020000 | Volume: 2.000000
Price: 67416.010000 | Volume: 2.000180
Price: 67416.000000 | Volume: 2.045810
Price: 67415.990000 | Volume: 2.341220
Price: 67415.980000 | Volume: 2.341320
Price: 67415.970000 | Volume: 7.341320
=====================

===== SUPPORT/RESISTANCE LEVELS =====
Support (Buy Wall) at Price: 67416.020000 | Qty: 2.000000
Support (Buy Wall) at Price: 67415.970000 | Qty: 5.000000
Resistance (Sell Wall) at Price: 67416.040000 | Qty: 2.562760
Resistance (Sell Wall) at Price: 67416.140000 | Qty: 1.000000
Resistance (Sell Wall) at Price: 67416.150000 | Qty: 1.000000
=====================
📡 ORDER EVENT: ID 2 Status: FILLED
✅ Order ID 2 cancelled
Validating: price=0.000000, qty=5.000000, type=1
Validation passed
✅ Order Placed [ID:17]: buy 5.000000 @ N/A (MARKET) Client: Client11 Latency: 5000ns
📡 TRADE EVENT: 2.562760 @ 67416.040000
💰 MARKET TRADE: 2.562760 @ 67416.040000 (Fee: 345.54)
📡 ORDER EVENT: ID 6 Status: FILLED
📡 TRADE EVENT: 0.00 @ 67416.06
💰 MARKET TRADE: 0.000180 @ 67416.060000 (Fee: 0.02)
📡 ORDER EVENT: ID 7 Status: FILLED
📡 TRADE EVENT: 0.24 @ 67416.07
💰 MARKET TRADE: 0.238830 @ 67416.070000 (Fee: 32.20)
📡 ORDER EVENT: ID 8 Status: FILLED
📡 TRADE EVENT: 0.00 @ 67416.08
💰 MARKET TRADE: 0.000100 @ 67416.080000 (Fee: 0.01)
📡 ORDER EVENT: ID 9 Status: FILLED
📡 TRADE EVENT: 0.00 @ 67416.11
💰 MARKET TRADE: 0.000100 @ 67416.110000 (Fee: 0.01)
📡 ORDER EVENT: ID 10 Status: FILLED
📡 TRADE EVENT: 0.50 @ 67416.12
💰 MARKET TRADE: 0.500000 @ 67416.120000 (Fee: 67.42)
📡 ORDER EVENT: ID 11 Status: FILLED
📡 TRADE EVENT: 0.50 @ 67416.13
💰 MARKET TRADE: 0.500000 @ 67416.130000 (Fee: 67.42)
📡 ORDER EVENT: ID 12 Status: FILLED
📡 TRADE EVENT: 1.00 @ 67416.14
💰 MARKET TRADE: 1.000000 @ 67416.140000 (Fee: 134.83)
📡 ORDER EVENT: ID 14 Status: FILLED
📡 TRADE EVENT: 0.20 @ 67416.15
💰 MARKET TRADE: 0.198030 @ 67416.150000 (Fee: 26.70)
📡 ORDER EVENT: ID 15 Status: PARTIAL
📡 ORDER EVENT: ID 17 Status: FILLED
Market Order Executed [ID:17]: Avg Price: 67416.08

===== ORDER BOOK =====
Spread: 0.13 | Mid: 67416.08
Price(USDT)	Amount(BTC)	Total(USDT)
ASKS (Sell) [Red in UI]
67416.15		0.801970		54.065730K
---------------------
BIDS (Buy) [Green in UI]
67416.020000		2.000000		134.832040K
67416.000000		0.045630		137.908232K
67415.990000		0.295410		157.823590K
67415.980000		0.000100		157.830331K
67415.970000		5.000000		494.910181K
=====================

===== DEPTH CHART SIMULATION =====
ASKS (Sell Orders Volume) [Red in UI]
Price: 67416.15 | Volume: 0.801970
---------------------
BIDS (Buy Orders Volume) [Green in UI]
Price: 67416.020000 | Volume: 2.000000
Price: 67416.000000 | Volume: 2.045630
Price: 67415.990000 | Volume: 2.341040
Price: 67415.980000 | Volume: 2.341140
Price: 67415.970000 | Volume: 7.341140
=====================

===== SUPPORT/RESISTANCE LEVELS =====
Support (Buy Wall) at Price: 67416.020000 | Qty: 2.000000
Support (Buy Wall) at Price: 67415.970000 | Qty: 5.000000
=====================

===== MATCHED TRADES =====
Timestamp: 2025-03-27 23:26:14 | Price: 67416.040000 | Qty: 2.562760 | BuyID: 17 | SellID: 6 | Fee: 345.54
Timestamp: 2025-03-27 23:26:14 | Price: 67416.06 | Qty: 0.000180 | BuyID: 17 | SellID: 7 | Fee: 0.02
Timestamp: 2025-03-27 23:26:14 | Price: 67416.07 | Qty: 0.238830 | BuyID: 17 | SellID: 8 | Fee: 32.20
Timestamp: 2025-03-27 23:26:14 | Price: 67416.08 | Qty: 0.000100 | BuyID: 17 | SellID: 9 | Fee: 0.01
Timestamp: 2025-03-27 23:26:14 | Price: 67416.11 | Qty: 0.000100 | BuyID: 17 | SellID: 10 | Fee: 0.01
Timestamp: 2025-03-27 23:26:14 | Price: 67416.12 | Qty: 0.500000 | BuyID: 17 | SellID: 11 | Fee: 67.42
Timestamp: 2025-03-27 23:26:14 | Price: 67416.13 | Qty: 0.500000 | BuyID: 17 | SellID: 12 | Fee: 67.42
Timestamp: 2025-03-27 23:26:14 | Price: 67416.14 | Qty: 1.000000 | BuyID: 17 | SellID: 14 | Fee: 134.83
Timestamp: 2025-03-27 23:26:14 | Price: 67416.15 | Qty: 0.198030 | BuyID: 17 | SellID: 15 | Fee: 26.70
========================

===== ORDER STATUS =====
ID: 1 | Side: buy | Price: 67416.03 | Qty: 1.047600 | Filled: 0.000000 | Latency: 5000ns
Status: CANCELLED
=====================
❌ Client Client1 has no position

===== POSITION =====
Client: Client6 | Qty: -2.562760 | Avg Price: 67416.040000
=====================
Best Bid: 67416.020000 | Best Ask: 67416.150000
✅ Snapshot saved to orderbook.snapshot
✅ Snapshot loaded from orderbook.snapshot

===== ORDER BOOK =====
Spread: 0.13 | Mid: 67416.08
Price(USDT)	Amount(BTC)	Total(USDT)
ASKS (Sell) [Red in UI]
67416.15		0.801970		54.065730K
---------------------
BIDS (Buy) [Green in UI]
67416.020000		2.000000		134.832040K
67416.000000		0.045630		137.908232K
67415.990000		0.295410		157.823590K
67415.980000		0.000100		157.830331K
67415.970000		5.000000		494.910181K
=====================




*/



