// OrderBook.cpp
// USES THE FIFO ORDER MATCHING ALGORITHM - FIRST IN FIRST OUT
// All declarations and definitions in a single file

// OrderBook.cpp
// USES THE FIFO ORDER MATCHING ALGORITHM - FIRST IN FIRST OUT
// All declarations and definitions in a single file

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

std::string TICKER = "BTC"; // ticker for the crypto we are trading

struct Balances {
    std::unordered_map<std::string, double> balance; // STORES USD : VALUE, BTC : VALUE 
    Balances() {
        balance["USD"] = 0;
        balance[TICKER] = 0;
    }
    Balances(std::string market, double value) {
        balance[market] = value;
    }

    std::string addBalance(std::string market, double value) {
        if (balance.find(market) != balance.end()) {
            balance[market] += value;
            return "Balance added successfully";
        }
        balance[market] = value;
        return "Balance added successfully";
    }
};

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

struct Order {
    std::string user_name;
    std::string side;
    double price;
    double quantity;
    long long order_id; // Unique order identifier
    static long long order_counter; // Global counter for all orders
    static int order_counter_bid;
    static int order_counter_ask;
    int insertion_order_bid;
    int insertion_order_ask;

    Order(std::string Username, std::string Side, double Price, double Quantity) {
        user_name = Username;
        side = Side;
        price = Price;
        quantity = Quantity;
        order_id = order_counter++; // Assign unique ID
        if (side == "bid") {
            insertion_order_bid = order_counter_bid++;
            insertion_order_ask = 0;
        } else {
            insertion_order_ask = order_counter_ask++; // Fixed typo here
            insertion_order_bid = 0;
        }
    }
};

long long Order::order_counter = 0; // Initialize global order counter
int Order::order_counter_bid = 0;
int Order::order_counter_ask = 0;

struct Trade {
    std::string buyer;
    std::string seller;
    double price;
    double quantity;
    long long timestamp; // Simple timestamp (could use time_t in practice)
    Trade(std::string b, std::string s, double p, double q) 
        : buyer(b), seller(s), price(p), quantity(q), timestamp(time(nullptr)) {}
};

class OrderBook {
private:
    std::vector<Order> bids;
    std::vector<Order> asks;
    std::unordered_map<std::string, User> users;
    std::vector<Trade> trade_history; // Trade log

    void flipBalance(const std::string& userId1, const std::string& userId2, double quantity, double price) {
        if (users.find(userId1) != users.end() && users.find(userId2) != users.end()) {
            if (users[userId1].user_balance.balance["USD"] >= price * quantity) {
                if (users[userId2].user_balance.balance[TICKER] >= quantity) {
                    users[userId1].user_balance.balance["USD"] -= price * quantity;
                    users[userId1].user_balance.balance[TICKER] += quantity;
                    users[userId2].user_balance.balance["USD"] += price * quantity;
                    users[userId2].user_balance.balance[TICKER] -= quantity;
                    cout << "Funds and BTC transferred!" << endl;
                    // Log the trade
                    trade_history.emplace_back(userId1, userId2, price, quantity);
                } else {
                    cout << "User does not have enough BTC to sell" << endl;
                }
            } else {
                cout << "User does not have enough USD to buy BTC" << endl;
            }
        } else {
            cout << "One or both users not found" << endl;
        }
    }

public:
    OrderBook() {
        // Initialize users with sufficient balances
        Balances balance1("USD", 10000000);
        balance1.addBalance(TICKER, 100);
        User marketMaker1("MarketMaker1", balance1);
        users["MarketMaker1"] = marketMaker1;

        Balances balance2("USD", 10000000);
        balance2.addBalance(TICKER, 100);
        User marketMaker2("MarketMaker2", balance2);
        users["MarketMaker2"] = marketMaker2;

        // Initialize asks (sell orders)
        asks.push_back(Order("MarketMaker1", "ask", 85924.96, 0.00006));
        asks.push_back(Order("MarketMaker1", "ask", 85924.54, 0.00006));
        asks.push_back(Order("MarketMaker1", "ask", 85924.52, 0.00039));
        asks.push_back(Order("MarketMaker1", "ask", 85924.19, 0.30604));
        asks.push_back(Order("MarketMaker1", "ask", 85924.18, 0.00014));
        asks.push_back(Order("MarketMaker1", "ask", 85924.00, 0.09517));
        asks.push_back(Order("MarketMaker1", "ask", 85923.99, 0.00014));
        asks.push_back(Order("MarketMaker1", "ask", 85923.98, 0.00006));
        asks.push_back(Order("MarketMaker1", "ask", 85923.02, 0.0480));
        asks.push_back(Order("MarketMaker1", "ask", 85923.00, 0.0400));
        asks.push_back(Order("MarketMaker1", "ask", 85922.90, 0.0440));
        asks.push_back(Order("MarketMaker1", "ask", 85922.88, 0.0520));
        asks.push_back(Order("MarketMaker1", "ask", 85922.78, 0.0400));
        asks.push_back(Order("MarketMaker1", "ask", 85922.75, 0.07510));
        asks.push_back(Order("MarketMaker1", "ask", 85922.74, 0.00014));
        asks.push_back(Order("MarketMaker1", "ask", 85922.67, 0.00041));
        asks.push_back(Order("MarketMaker1", "ask", 85922.66, 1.77704));

        // Initialize bids (buy orders)
        bids.push_back(Order("MarketMaker2", "bid", 85921.74, 3.80013));
        bids.push_back(Order("MarketMaker2", "bid", 85921.67, 0.00007));
        bids.push_back(Order("MarketMaker2", "bid", 85921.58, 0.01326));
        bids.push_back(Order("MarketMaker2", "bid", 85921.57, 4.01376));
        bids.push_back(Order("MarketMaker2", "bid", 85921.50, 0.49514));
        bids.push_back(Order("MarketMaker2", "bid", 85921.35, 0.00007));
        bids.push_back(Order("MarketMaker2", "bid", 85921.24, 0.00096));
        bids.push_back(Order("MarketMaker2", "bid", 85921.23, 0.01328));
        bids.push_back(Order("MarketMaker2", "bid", 85921.16, 0.00259));
        bids.push_back(Order("MarketMaker2", "bid", 85921.09, 0.00007));
        bids.push_back(Order("MarketMaker2", "bid", 85921.08, 0.34329));
        bids.push_back(Order("MarketMaker2", "bid", 85920.82, 0.00013));
        bids.push_back(Order("MarketMaker2", "bid", 85920.00, 0.09528));
        bids.push_back(Order("MarketMaker2", "bid", 85919.69, 0.07804));
        bids.push_back(Order("MarketMaker2", "bid", 85919.49, 0.19946));
        bids.push_back(Order("MarketMaker2", "bid", 85919.20, 0.04656));
        bids.push_back(Order("MarketMaker2", "bid", 85919.00, 0.33926));
    }

    ~OrderBook() {}

    std::string makeUser(std::string Username) {
        User user(Username);
        users[Username] = user;
        cout << "User: " << Username << " created successfully for BTC trading" << endl;
        return "User created successfully";
    }

    std::string add_bid(std::string Username, double Price, double Quantity) {
        double remQty = Quantity;
        std::stable_sort(asks.begin(), asks.end(), [](const Order &a, const Order &b) {
            if (a.price == b.price) return a.insertion_order_ask < b.insertion_order_ask;
            return a.price < b.price;
        });

        for (auto it = asks.begin(); it != asks.end(); /* no increment here */) {
            if (remQty > 0 && Price >= it->price) {
                if (it->quantity > remQty) {
                    it->quantity -= remQty;
                    flipBalance(Username, it->user_name, remQty, it->price);
                    cout << "Bid Satisfied Successfully at price: " << it->price << " and quantity: " << remQty << " BTC" << endl;
                    remQty = 0;
                    break;
                } else {
                    remQty -= it->quantity;
                    flipBalance(Username, it->user_name, it->quantity, it->price);
                    cout << "Bid Satisfied Partially at price: " << it->price << " and quantity: " << it->quantity << " BTC" << endl;
                    it = asks.erase(it);
                }
            } else {
                ++it;
            }
        }

        if (remQty > 0) {
            Order bid(Username, "bid", Price, remQty);
            bids.push_back(bid);
            cout << "Remaining quantity of bids added to Orderbook (Order ID: " << bid.order_id << ")" << endl;
        }

        if (remQty == 0) {
            cout << "Complete Bid Satisfied Successfully" << endl;
        }
        return "Bid added/satisfied successfully.";
    }

    std::string add_ask(std::string Username, double Price, double Quantity) {
        double remQty = Quantity;
        std::stable_sort(bids.begin(), bids.end(), [](const Order &a, const Order &b) {
            if (a.price == b.price) return a.insertion_order_bid < b.insertion_order_bid;
            return a.price > b.price;
        });

        for (auto it = bids.begin(); it != bids.end(); /* no increment here */) {
            if (remQty > 0 && Price <= it->price) {
                if (it->quantity > remQty) {
                    it->quantity -= remQty;
                    flipBalance(it->user_name, Username, remQty, it->price);
                    cout << "Ask Satisfied Successfully at price: " << it->price << " and quantity: " << remQty << " BTC" << endl;
                    remQty = 0;
                    break;
                } else {
                    remQty -= it->quantity;
                    flipBalance(it->user_name, Username, it->quantity, it->price);
                    cout << "Ask Satisfied Partially at price: " << it->price << " and quantity: " << it->quantity << " BTC" << endl;
                    it = bids.erase(it);
                }
            } else {
                it++;
            }
        }

        if (remQty > 0) {
            Order ask(Username, "ask", Price, remQty);
            asks.push_back(ask);
            cout << "Remaining quantity of asks added to Orderbook (Order ID: " << ask.order_id << ")" << endl;
        }

        if (remQty == 0) {
            cout << "Complete Ask Satisfied Successfully" << endl;
        }
        return "Ask added successfully.";
    }

    std::string add_market_bid(std::string Username, double Quantity) {
        double remQty = Quantity;
        std::stable_sort(asks.begin(), asks.end(), [](const Order &a, const Order &b) {
            return a.price < b.price; // Best price first
        });

        for (auto it = asks.begin(); it != asks.end() && remQty > 0;) {
            if (it->quantity > remQty) {
                it->quantity -= remQty;
                flipBalance(Username, it->user_name, remQty, it->price);
                cout << "Market Bid Satisfied at price: " << it->price << " and quantity: " << remQty << " BTC" << endl;
                remQty = 0;
            } else {
                remQty -= it->quantity;
                flipBalance(Username, it->user_name, it->quantity, it->price);
                cout << "Market Bid Satisfied Partially at price: " << it->price << " and quantity: " << it->quantity << " BTC" << endl;
                it = asks.erase(it);
            }
        }

        if (remQty > 0) {
            cout << "Insufficient liquidity to fill market bid. Remaining: " << remQty << " BTC" << endl;
        } else {
            cout << "Market Bid Filled Successfully" << endl;
        }
        return "Market bid processed.";
    }

    std::string add_market_ask(std::string Username, double Quantity) {
        double remQty = Quantity;
        std::stable_sort(bids.begin(), bids.end(), [](const Order &a, const Order &b) {
            return a.price > b.price; // Best price first
        });

        for (auto it = bids.begin(); it != bids.end() && remQty > 0;) {
            if (it->quantity > remQty) {
                it->quantity -= remQty;
                flipBalance(it->user_name, Username, remQty, it->price);
                cout << "Market Ask Satisfied at price: " << it->price << " and quantity: " << remQty << " BTC" << endl;
                remQty = 0;
            } else {
                remQty -= it->quantity;
                flipBalance(it->user_name, Username, it->quantity, it->price);
                cout << "Market Ask Satisfied Partially at price: " << it->price << " and quantity: " << it->quantity << " BTC" << endl;
                it = bids.erase(it);
            }
        }

        if (remQty > 0) {
            cout << "Insufficient liquidity to fill market ask. Remaining: " << remQty << " BTC" << endl;
        } else {
            cout << "Market Ask Filled Successfully" << endl;
        }
        return "Market ask processed.";
    }

    std::string getBalance(std::string username) {
        if (users.find(username) != users.end()) {
            cout << "User found" << endl;
            cout << "User balance is as follows: " << endl;
            for (auto it = users[username].user_balance.balance.begin(); it != users[username].user_balance.balance.end(); ++it) {
                cout << it->first << " : " << it->second << endl;
            }
            return "Balance retrieved successfully.";
        } else {
            cout << "User not found!!" << endl;
            return "User not found";
        }
    }

    std::string getQuote(double qty) {
        std::stable_sort(asks.begin(), asks.end(), [](const Order &a, const Order &b) {
            if (a.price == b.price) return a.insertion_order_ask < b.insertion_order_ask;
            return a.price < b.price;
        });

        for (auto it = asks.begin(); it != asks.end(); ++it) {
            if (qty > 0 && qty <= it->quantity) {
                cout << TICKER << "-> Quantity available: " << qty << " BTC at " << it->price << " USD" << endl;
                return "Quote retrieved successfully.";
            } else if (qty > 0 && qty > it->quantity) {
                cout << TICKER << "-> Quantity available: " << it->quantity << " BTC at " << it->price << " USD" << endl;
                qty -= it->quantity;
            } else {
                return "Quote retrieved successfully.";
            }
        }
        cout << "Quote retrieved successfully." << endl;
        return "Quote retrieved successfully.";
    }

    std::string getDepth() {
        std::sort(asks.begin(), asks.end(), [](const Order &a, const Order &b) {
            return a.price > b.price;
        });
        std::sort(bids.begin(), bids.end(), [](const Order &a, const Order &b) {
            return a.price > b.price;
        });

        cout << "Order Book\n\n";
        cout << setw(15) << left << "Price(USDT)" << setw(15) << "Amount(BTC)" << setw(15) << "TOTAL" << endl;

        cout << "\nASK\n";
        for (const auto &ask : asks) {
            cout << fixed << setprecision(2) << setw(15) << left << ask.price;
            cout << setprecision(5) << setw(15) << ask.quantity;
            cout << setprecision(2);
            double total = ask.price * ask.quantity;
            cout << setw(15) << (total < 1000 ? to_string(total) : to_string(total / 1000) + "K") << endl;
        }

        cout << "\nBID\n";
        for (const auto &bid : bids) {
            cout << fixed << setprecision(2) << setw(15) << left << bid.price;
            cout << setprecision(5) << setw(15) << bid.quantity;
            cout << setprecision(2);
            double total = bid.price * bid.quantity;
            cout << setw(15) << (total < 1000 ? to_string(total) : to_string(total / 1000) + "K") << endl;
        }

        return "Order book displayed";
    }

    std::string addBalanace(std::string Username, std::string market, double value) {
        if (users.find(Username) != users.end()) {
            users[Username].user_balance.addBalance(market, value);
            cout << "Balance added successfully" << endl;
            return "Balance added successfully";
        }
        cout << "User not found!! Please enter the right Username to add balance!" << endl;
        return "User not found";
    }

    void cancelAsk(std::string Username, long long OrderId = -1, double Price = 0, double Quantity = 0) {
        for (auto it = asks.begin(); it != asks.end(); ++it) {
            if (OrderId != -1 && it->order_id == OrderId && it->user_name == Username) {
                asks.erase(it);
                cout << "Ask cancelled successfully (Order ID: " << OrderId << ")" << endl;
                return;
            } else if (OrderId == -1 && it->user_name == Username && it->price == Price) {
                if (it->quantity == Quantity) {
                    asks.erase(it);
                    cout << "Ask cancelled successfully" << endl;
                    return;
                } else if (it->quantity > Quantity) {
                    it->quantity -= Quantity;
                    cout << "Ask partially cancelled successfully" << endl;
                    return;
                } else {
                    cout << "Ask quantity is less than the quantity you want to cancel" << endl;
                    return;
                }
            }
        }
        cout << "Ask not found!! Please enter the right Username, Order ID, Price, and Quantity!" << endl;
    }

    void cancelBid(std::string Username, long long OrderId = -1, double Price = 0, double Quantity = 0) {
        for (auto it = bids.begin(); it != bids.end(); ++it) {
            if (OrderId != -1 && it->order_id == OrderId && it->user_name == Username) {
                bids.erase(it);
                cout << "Bid cancelled successfully (Order ID: " << OrderId << ")" << endl;
                return;
            } else if (OrderId == -1 && it->user_name == Username && it->price == Price) {
                if (it->quantity == Quantity) {
                    bids.erase(it);
                    cout << "Bid cancelled successfully" << endl;
                    return;
                } else if (it->quantity > Quantity) {
                    it->quantity -= Quantity;
                    cout << "Bid partially cancelled successfully" << endl;
                    return;
                } else {
                    cout << "Bid quantity is less than the quantity you want to cancel" << endl;
                    return;
                }
            }
        }
        cout << "Bid not found!! Please enter the right Username, Order ID, Price, and Quantity!" << endl;
    }

    std::string getTradeHistory() {
        if (trade_history.empty()) {
            cout << "No trades have occurred yet." << endl;
            return "No trades";
        }
        cout << "Trade History\n\n";
        cout << setw(15) << left << "Buyer" << setw(15) << "Seller" << setw(15) << "Price" << setw(15) << "Quantity" << "Timestamp" << endl;
        for (const auto& trade : trade_history) {
            cout << setw(15) << left << trade.buyer << setw(15) << trade.seller 
                 << setw(15) << fixed << setprecision(2) << trade.price 
                 << setw(15) << setprecision(5) << trade.quantity 
                 << trade.timestamp << endl;
        }
        return "Trade history displayed";
    }

    std::string getSpread() {
        if (bids.empty() || asks.empty()) {
            cout << "Insufficient data to calculate spread." << endl;
            return "No spread available";
        }
        std::stable_sort(bids.begin(), bids.end(), [](const Order &a, const Order &b) {
            return a.price > b.price;
        });
        std::stable_sort(asks.begin(), asks.end(), [](const Order &a, const Order &b) {
            return a.price < b.price;
        });
        double best_bid = bids.front().price;
        double best_ask = asks.front().price;
        double spread = best_ask - best_bid;
        cout << "Best Bid: " << best_bid << " | Best Ask: " << best_ask << " | Spread: " << spread << " USD" << endl;
        return "Spread calculated";
    }
};

int main() {
    OrderBook EXCH;
    int choice;
    string username;
    double amount, price;
    long long order_id;
    string currency;

    cout << "\n=========== WELCOME TO THE " << TICKER << " MARKET AND HAPPY TRADING ===========\n\n";
    cout << "\n=========== INITIAL BTC MARKET PRICES ===========\n";
    EXCH.getDepth();

    while (true) {
        cout << "\n=========== " << TICKER << " Trading Platform ===========\n\n";
        cout << "1. Sign Up User\n";
        cout << "2. Add Balance to User Account\n";
        cout << "3. Check Current Market Prices\n";
        cout << "4. Add Limit Bid\n";
        cout << "5. Add Limit Ask (Sell)\n";
        cout << "6. Add Market Bid\n";
        cout << "7. Add Market Ask (Sell)\n";
        cout << "8. Get Current Quote\n";
        cout << "9. Check User Balance\n";
        cout << "10. Cancel Bid\n";
        cout << "11. Cancel Ask\n";
        cout << "12. View Trade History\n";
        cout << "13. View Bid-Ask Spread\n";
        cout << "14. Exit\n\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: // Sign Up User
                cout << "Enter username: ";
                cin >> username;
                EXCH.makeUser(username);
                break;

            case 2: // Add Balance
                cout << "Enter username: ";
                cin >> username;
                cout << "Enter currency (USD/BTC): ";
                cin >> currency;
                cout << "Enter amount: ";
                cin >> amount;
                EXCH.addBalanace(username, currency, amount);
                break;

            case 3: // Check Market Prices
                cout << "\n=========== CURRENT MARKET PRICES ===========\n";
                EXCH.getDepth();
                break;

            case 4: // Add Limit Bid
                cout << "Enter username: ";
                cin >> username;
                cout << "Enter bid price: ";
                cin >> price;
                cout << "Enter amount: ";
                cin >> amount;
                EXCH.add_bid(username, price, amount);
                break;

            case 5: // Add Limit Ask
                cout << "Enter username: ";
                cin >> username;
                cout << "Enter ask price: ";
                cin >> price;
                cout << "Enter amount: ";
                cin >> amount;
                EXCH.add_ask(username, price, amount);
                break;

            case 6: // Add Market Bid
                cout << "Enter username: ";
                cin >> username;
                cout << "Enter amount: ";
                cin >> amount;
                EXCH.add_market_bid(username, amount);
                break;

            case 7: // Add Market Ask
                cout << "Enter username: ";
                cin >> username;
                cout << "Enter amount: ";
                cin >> amount;
                EXCH.add_market_ask(username, amount);
                break;

            case 8: // Get Quote
                cout << "Enter amount of BTC to quote: ";
                cin >> amount;
                EXCH.getQuote(amount);
                break;

            case 9: // Check Balance
                cout << "Enter username: ";
                cin >> username;
                EXCH.getBalance(username);
                break;

            case 10: // Cancel Bid
                cout << "Enter username: ";
                cin >> username;
                cout << "Cancel by Order ID? (1 = Yes, 0 = No): ";
                int use_id;
                cin >> use_id;
                if (use_id) {
                    cout << "Enter Order ID: ";
                    cin >> order_id;
                    EXCH.cancelBid(username, order_id);
                } else {
                    cout << "Enter bid price: ";
                    cin >> price;
                    cout << "Enter amount: ";
                    cin >> amount;
                    EXCH.cancelBid(username, -1, price, amount);
                }
                break;

            case 11: // Cancel Ask
                cout << "Enter username: ";
                cin >> username;
                cout << "Cancel by Order ID? (1 = Yes, 0 = No): ";
                cin >> use_id;
                if (use_id) {
                    cout << "Enter Order ID: ";
                    cin >> order_id;
                    EXCH.cancelAsk(username, order_id);
                } else {
                    cout << "Enter ask price: ";
                    cin >> price;
                    cout << "Enter amount: ";
                    cin >> amount;
                    EXCH.cancelAsk(username, -1, price, amount);
                }
                break;

            case 12: // View Trade History
                EXCH.getTradeHistory();
                break;

            case 13: // View Spread
                EXCH.getSpread();
                break;

            case 14: // Exit
                cout << "\nThank you for trading. Goodbye!\n";
                return 0;

            default:
                cout << "Invalid choice. Please try again.\n";
                break;
        }
    }

    return 0;
}



/*
Output:sample

=========== WELCOME TO THE BTC MARKET AND HAPPY TRADING ===========


=========== INITIAL BTC MARKET PRICES ===========
Order Book

Price(USDT)    Amount(BTC)    TOTAL

ASK
85924.96       0.00006        5.155498       
85924.54       0.00006        5.155472       
85924.52       0.00039        33.510563      
85924.19       0.30604        26.296239K     
85924.18       0.00014        12.029385      
85924.00       0.09517        8.177387K      
85923.99       0.00014        12.029359      
85923.98       0.00006        5.155439       
85923.02       0.04800        4.124305K      
85923.00       0.04000        3.436920K      
85922.90       0.04400        3.780608K
85922.88       0.05200        4.467990K
85922.78       0.04000        3.436911K
85922.75       0.07510        6.452799K      
85922.74       0.00014        12.029184
85922.67       0.00041        35.228295
85922.66       1.77704        152.688004K

BID
85921.74       3.80013        326.513782K
85921.67       0.00007        6.014517
85921.58       0.01326        1.139320K      
85921.57       4.01376        344.868561K
85921.50       0.49514        42.543172K
85921.35       0.00007        6.014494
85921.24       0.00096        82.484390
85921.23       0.01328        1.141034K      
85921.16       0.00259        222.535804
85921.09       0.00007        6.014476
85921.08       0.34329        29.495848K
85920.82       0.00013        11.169707
85920.00       0.09528        8.186458K
85919.69       0.07804        6.705173K
85919.49       0.19946        17.137501K
85919.20       0.04656        4.000398K
85919.00       0.33926        29.148880K     

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 8 
Enter amount of BTC to quote: 85921.10
BTC-> Quantity available: 1.78 BTC at 85922.66 USD
BTC-> Quantity available: 0.00 BTC at 85922.67 USD
BTC-> Quantity available: 0.00 BTC at 85922.74 USD
BTC-> Quantity available: 0.08 BTC at 85922.75 USD
BTC-> Quantity available: 0.04 BTC at 85922.78 USD
BTC-> Quantity available: 0.05 BTC at 85922.88 USD
BTC-> Quantity available: 0.04 BTC at 85922.90 USD
BTC-> Quantity available: 0.04 BTC at 85923.00 USD
BTC-> Quantity available: 0.05 BTC at 85923.02 USD
BTC-> Quantity available: 0.00 BTC at 85923.98 USD
BTC-> Quantity available: 0.00 BTC at 85923.99 USD
BTC-> Quantity available: 0.10 BTC at 85924.00 USD
BTC-> Quantity available: 0.00 BTC at 85924.18 USD
BTC-> Quantity available: 0.31 BTC at 85924.19 USD
BTC-> Quantity available: 0.00 BTC at 85924.52 USD
BTC-> Quantity available: 0.00 BTC at 85924.54 USD
BTC-> Quantity available: 0.00 BTC at 85924.96 USD
Quote retrieved successfully.

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 12
No trades have occurred yet.

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 2
Enter username: pranay
Enter currency (USD/BTC): 100000
Enter amount: 100000
User not found!! Please enter the right Username to add balance!

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 2      
Enter username: pranay
Enter currency (USD/BTC): USD
Enter amount: 100000
User not found!! Please enter the right Username to add balance!

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 1
Enter username: pranay
User: pranay created successfully for BTC trading

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 2
Enter username: pranay
Enter currency (USD/BTC): USD
Enter amount: 100000
Balance added successfully

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 3

=========== CURRENT MARKET PRICES ===========
Order Book

Price(USDT)    Amount(BTC)    TOTAL

ASK
85924.96       0.00006        5.155498       
85924.54       0.00006        5.155472
85924.52       0.00039        33.510563
85924.19       0.30604        26.296239K
85924.18       0.00014        12.029385
85924.00       0.09517        8.177387K
85923.99       0.00014        12.029359      
85923.98       0.00006        5.155439
85923.02       0.04800        4.124305K
85923.00       0.04000        3.436920K
85922.90       0.04400        3.780608K      
85922.88       0.05200        4.467990K
85922.78       0.04000        3.436911K
85922.75       0.07510        6.452799K      
85922.74       0.00014        12.029184
85922.67       0.00041        35.228295
85922.66       1.77704        152.688004K    

BID
85921.74       3.80013        326.513782K
85921.67       0.00007        6.014517       
85921.58       0.01326        1.139320K
85921.57       4.01376        344.868561K
85921.50       0.49514        42.543172K     
85921.35       0.00007        6.014494
85921.24       0.00096        82.484390
85921.23       0.01328        1.141034K
85921.16       0.00259        222.535804     
85921.09       0.00007        6.014476
85921.08       0.34329        29.495848K
85920.82       0.00013        11.169707
85920.00       0.09528        8.186458K      
85919.69       0.07804        6.705173K
85919.49       0.19946        17.137501K
85919.20       0.04656        4.000398K      
85919.00       0.33926        29.148880K

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 4
Enter username: pranay
Enter bid price: 85921
Enter amount: 0.14
Remaining quantity of bids added to Orderbook (Order ID: 34)

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 12
No trades have occurred yet.

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 3

=========== CURRENT MARKET PRICES ===========
Order Book

Price(USDT)    Amount(BTC)    TOTAL

ASK
85924.96       0.00006        5.155498
85924.54       0.00006        5.155472
85924.52       0.00039        33.510563      
85924.19       0.30604        26.296239K
85924.18       0.00014        12.029385
85924.00       0.09517        8.177387K      
85923.99       0.00014        12.029359
85923.98       0.00006        5.155439
85923.02       0.04800        4.124305K
85923.00       0.04000        3.436920K      
85922.90       0.04400        3.780608K
85922.88       0.05200        4.467990K
85922.78       0.04000        3.436911K
85922.75       0.07510        6.452799K      
85922.74       0.00014        12.029184
85922.67       0.00041        35.228295
85922.66       1.77704        152.688004K

BID
85921.74       3.80013        326.513782K
85921.67       0.00007        6.014517
85921.58       0.01326        1.139320K
85921.57       4.01376        344.868561K    
85921.50       0.49514        42.543172K
85921.35       0.00007        6.014494       
85921.24       0.00096        82.484390
85921.23       0.01328        1.141034K
85921.16       0.00259        222.535804
85921.09       0.00007        6.014476
85921.08       0.34329        29.495848K
85921.00       0.14000        12.028940K
85920.82       0.00013        11.169707      
85920.00       0.09528        8.186458K
85919.69       0.07804        6.705173K      
85919.49       0.19946        17.137501K
85919.20       0.04656        4.000398K
85919.00       0.33926        29.148880K

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
85919.20       0.04656        4.000398K
85919.00       0.33926        29.148880K

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 4
Enter username: pranay
Enter bid price: 85930
Enter amount: 0.15
Funds and BTC transferred!
Bid Satisfied Successfully at price: 85922.66 and quantity: 0.15 BTC
Complete Bid Satisfied Successfully

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 12
Trade History

Buyer          Seller         Price          Quantity       Timestamp
pranay         MarketMaker1   85922.66       0.15000        1743097832

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 13
Best Bid: 85921.74000 | Best Ask: 85922.66000 | Spread: 0.92000 USD

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 10
Enter username: pranay
Cancel by Order ID? (1 = Yes, 0 = No): 1
Enter Order ID: 34
Bid cancelled successfully (Order ID: 34)

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 9
Enter username: pranay
User found
User balance is as follows:
BTC : 0.15000
USD : 87111.60100

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 8
Enter amount of BTC to quote: 85934
BTC-> Quantity available: 1.62704 BTC at 85922.66000 USD
BTC-> Quantity available: 0.00041 BTC at 85922.67000 USD
BTC-> Quantity available: 0.00014 BTC at 85922.74000 USD
BTC-> Quantity available: 0.07510 BTC at 85922.75000 USD
BTC-> Quantity available: 0.04000 BTC at 85922.78000 USD
BTC-> Quantity available: 0.05200 BTC at 85922.88000 USD
BTC-> Quantity available: 0.04400 BTC at 85922.90000 USD
BTC-> Quantity available: 0.04000 BTC at 85923.00000 USD
BTC-> Quantity available: 0.04800 BTC at 85923.02000 USD
BTC-> Quantity available: 0.00006 BTC at 85923.98000 USD
BTC-> Quantity available: 0.00014 BTC at 85923.99000 USD
BTC-> Quantity available: 0.09517 BTC at 85924.00000 USD
BTC-> Quantity available: 0.00014 BTC at 85924.18000 USD
BTC-> Quantity available: 0.30604 BTC at 85924.19000 USD
BTC-> Quantity available: 0.00039 BTC at 85924.52000 USD
BTC-> Quantity available: 0.00006 BTC at 85924.54000 USD
BTC-> Quantity available: 0.00006 BTC at 85924.96000 USD
Quote retrieved successfully.

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice: 6
Enter username: pranay
Enter amount: 85924
User does not have enough USD to buy BTC
Market Bid Satisfied Partially at price: 85922.66000 and quantity: 1.62704 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85922.67000 and quantity: 0.00041 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85922.74000 and quantity: 0.00014 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85922.75000 and quantity: 0.07510 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85922.78000 and quantity: 0.04000 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85922.88000 and quantity: 0.05200 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85922.90000 and quantity: 0.04400 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85923.00000 and quantity: 0.04000 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85923.02000 and quantity: 0.04800 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85923.98000 and quantity: 0.00006 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85923.99000 and quantity: 0.00014 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85924.00000 and quantity: 0.09517 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85924.18000 and quantity: 0.00014 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85924.19000 and quantity: 0.30604 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85924.52000 and quantity: 0.00039 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85924.54000 and quantity: 0.00006 BTC
Funds and BTC transferred!
Market Bid Satisfied Partially at price: 85924.96000 and quantity: 0.00006 BTC
Insufficient liquidity to fill market bid. Remaining: 85921.67125 BTC

=========== BTC Trading Platform ===========

1. Sign Up User
2. Add Balance to User Account
3. Check Current Market Prices
4. Add Limit Bid
5. Add Limit Ask (Sell)
6. Add Market Bid
7. Add Market Ask (Sell)
8. Get Current Quote
9. Check User Balance
10. Cancel Bid
11. Cancel Ask
12. View Trade History
13. View Bid-Ask Spread
14. Exit

Enter your choice:
*/