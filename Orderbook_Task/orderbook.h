#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <string>
#include <unordered_map>
#include <vector>

std::string TICKER = "BTC"; // ticker for the crypto we are trading

struct Balances {
    std::unordered_map<std::string, int> balance; // STORES USD : VALUE, BTC : VALUE 
    Balances() {
        balance["USD"] = 0;
        balance[TICKER] = 0;
    }
    Balances(std::string market, int value) {
        balance[market] = value;
    }

    std::string addBalance(std::string market, int value) {
        if (balance.find(market) != balance.end()) {
            balance[market] += value;
            return "Balance added successfully";
        }
        balance[market] = value;
        return "Balance added successfully";
    }
};

// Moved User struct here, before OrderBook
struct User {
    std::string user_name;
    Balances user_balance;

    User() {}

    User(std::string Username, Balances b) {
        user_name = Username;
        user_balance = b;
    }

    User(std::string Username) {
        user_name = Username;
        user_balance = Balances();
    }
};

// Moved Order struct here, before OrderBook
struct Order {
    std::string user_name;
    std::string side;
    int price;
    int quantity;
    static int order_counter_bid; // Declare a static counter to keep track of the order number
    static int order_counter_ask; // Declare a static counter to keep track of the order number
    int insertion_order_bid; // order in which the order was inserted into the order book
    int insertion_order_ask; // order in which the order was inserted into the order book
    Order(std::string Username, std::string Side, int Price, int Quantity) {
        user_name = Username;
        side = Side;
        price = Price;
        quantity = Quantity;
        if (side == "bid") {
            insertion_order_bid = order_counter_bid++;
            insertion_order_ask = 0; // Initialize the other counter to avoid uninitialized use
        }
        else {
            insertion_order_ask = order_counter_ask++;
            insertion_order_bid = 0; // Initialize the other counter to avoid uninitialized use
        }
    }
};

class OrderBook {
private:
    std::vector<Order> bids; // Now Order is defined
    std::vector<Order> asks; // Now Order is defined
    std::unordered_map<std::string, User> users; // Now User is defined
    void flipBalance(const std::string& userId1, const std::string& userId2, double quantity, double price);

public:
    OrderBook(); // constructor
    ~OrderBook(); // destructor 
    std::string add_bid(std::string Username, int Price, int Quantity);
    std::string add_ask(std::string Username, int Price, int Quantity);
    std::string getBalance(std::string username);
    std::string getQuote(int qty);
    std::string getDepth();
    std::string makeUser(std::string);
    std::string addBalanace(std::string Username, std::string market, int value);
    void cancelBid(std::string Username, int Price, int Quantity);
    void cancelAsk(std::string Username, int Price, int Quantity);
};

#endif