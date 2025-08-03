import time
from collections import defaultdict
from operator import itemgetter
import sys

TICKER = "BTC"  # ticker for the crypto we are trading

class Balances:
    def __init__(self, market=None, value=0):
        self.balance = defaultdict(float)
        if market:
            self.balance[market] = value
        else:
            self.balance["USD"] = 0
            self.balance[TICKER] = 0

    def add_balance(self, market, value):
        self.balance[market] += value
        return "Balance added successfully"

class User:
    def __init__(self, username, balances=None):
        self.user_name = username
        self.user_balance = balances if balances else Balances()

class Order:
    order_counter = 0
    order_counter_bid = 0
    order_counter_ask = 0

    def __init__(self, username, side, price, quantity):
        self.user_name = username
        self.side = side
        self.price = price
        self.quantity = quantity
        self.order_id = Order.order_counter
        Order.order_counter += 1
        
        if side == "bid":
            self.insertion_order_bid = Order.order_counter_bid
            Order.order_counter_bid += 1
            self.insertion_order_ask = 0
        else:
            self.insertion_order_ask = Order.order_counter_ask
            Order.order_counter_ask += 1
            self.insertion_order_bid = 0

class Trade:
    def __init__(self, buyer, seller, price, quantity):
        self.buyer = buyer
        self.seller = seller
        self.price = price
        self.quantity = quantity
        self.timestamp = int(time.time())

class OrderBook:
    def __init__(self):
        self.bids = []
        self.asks = []
        self.users = {}
        self.trade_history = []

        # Initialize users with sufficient balances
        balance1 = Balances("USD", 10000000)
        balance1.add_balance(TICKER, 100)
        self.users["MarketMaker1"] = User("MarketMaker1", balance1)

        balance2 = Balances("USD", 10000000)
        balance2.add_balance(TICKER, 100)
        self.users["MarketMaker2"] = User("MarketMaker2", balance2)

        # Initialize asks (sell orders)
        initial_asks = [
            ("MarketMaker1", "ask", 85924.96, 0.00006),
            ("MarketMaker1", "ask", 85924.54, 0.00006),
            ("MarketMaker1", "ask", 85924.52, 0.00039),
            ("MarketMaker1", "ask", 85924.19, 0.30604),
            ("MarketMaker1", "ask", 85924.18, 0.00014),
            ("MarketMaker1", "ask", 85924.00, 0.09517),
            ("MarketMaker1", "ask", 85923.99, 0.00014),
            ("MarketMaker1", "ask", 85923.98, 0.00006),
            ("MarketMaker1", "ask", 85923.02, 0.0480),
            ("MarketMaker1", "ask", 85923.00, 0.0400),
            ("MarketMaker1", "ask", 85922.90, 0.0440),
            ("MarketMaker1", "ask", 85922.88, 0.0520),
            ("MarketMaker1", "ask", 85922.78, 0.0400),
            ("MarketMaker1", "ask", 85922.75, 0.07510),
            ("MarketMaker1", "ask", 85922.74, 0.00014),
            ("MarketMaker1", "ask", 85922.67, 0.00041),
            ("MarketMaker1", "ask", 85922.66, 1.77704),
        ]
        self.asks.extend(Order(*args) for args in initial_asks)

        # Initialize bids (buy orders)
        initial_bids = [
            ("MarketMaker2", "bid", 85921.74, 3.80013),
            ("MarketMaker2", "bid", 85921.67, 0.00007),
            ("MarketMaker2", "bid", 85921.58, 0.01326),
            ("MarketMaker2", "bid", 85921.57, 4.01376),
            ("MarketMaker2", "bid", 85921.50, 0.49514),
            ("MarketMaker2", "bid", 85921.35, 0.00007),
            ("MarketMaker2", "bid", 85921.24, 0.00096),
            ("MarketMaker2", "bid", 85921.23, 0.01328),
            ("MarketMaker2", "bid", 85921.16, 0.00259),
            ("MarketMaker2", "bid", 85921.09, 0.00007),
            ("MarketMaker2", "bid", 85921.08, 0.34329),
            ("MarketMaker2", "bid", 85920.82, 0.00013),
            ("MarketMaker2", "bid", 85920.00, 0.09528),
            ("MarketMaker2", "bid", 85919.69, 0.07804),
            ("MarketMaker2", "bid", 85919.49, 0.19946),
            ("MarketMaker2", "bid", 85919.20, 0.04656),
            ("MarketMaker2", "bid", 85919.00, 0.33926),
        ]
        self.bids.extend(Order(*args) for args in initial_bids)

    def flip_balance(self, user_id1, user_id2, quantity, price):
        if user_id1 in self.users and user_id2 in self.users:
            if self.users[user_id1].user_balance.balance["USD"] >= price * quantity:
                if self.users[user_id2].user_balance.balance[TICKER] >= quantity:
                    self.users[user_id1].user_balance.balance["USD"] -= price * quantity
                    self.users[user_id1].user_balance.balance[TICKER] += quantity
                    self.users[user_id2].user_balance.balance["USD"] += price * quantity
                    self.users[user_id2].user_balance.balance[TICKER] -= quantity
                    print("Funds and BTC transferred!")
                    self.trade_history.append(Trade(user_id1, user_id2, price, quantity))
                else:
                    print("User does not have enough BTC to sell")
            else:
                print("User does not have enough USD to buy BTC")
        else:
            print("One or both users not found")

    def make_user(self, username):
        self.users[username] = User(username)
        print(f"User: {username} created successfully for BTC trading")
        return "User created successfully"

    def add_bid(self, username, price, quantity):
        rem_qty = quantity
        self.asks.sort(key=lambda x: (x.price, x.insertion_order_ask))

        i = 0
        while i < len(self.asks) and rem_qty > 0 and price >= self.asks[i].price:
            if self.asks[i].quantity > rem_qty:
                self.asks[i].quantity -= rem_qty
                self.flip_balance(username, self.asks[i].user_name, rem_qty, self.asks[i].price)
                print(f"Bid Satisfied Successfully at price: {self.asks[i].price} and quantity: {rem_qty} BTC")
                rem_qty = 0
            else:
                rem_qty -= self.asks[i].quantity
                self.flip_balance(username, self.asks[i].user_name, self.asks[i].quantity, self.asks[i].price)
                print(f"Bid Satisfied Partially at price: {self.asks[i].price} and quantity: {self.asks[i].quantity} BTC")
                del self.asks[i]
                continue
            i += 1

        if rem_qty > 0:
            bid = Order(username, "bid", price, rem_qty)
            self.bids.append(bid)
            print(f"Remaining quantity of bids added to Orderbook (Order ID: {bid.order_id})")

        if rem_qty == 0:
            print("Complete Bid Satisfied Successfully")
        return "Bid added/satisfied successfully."

    def add_ask(self, username, price, quantity):
        rem_qty = quantity
        self.bids.sort(key=lambda x: (-x.price, x.insertion_order_bid))

        i = 0
        while i < len(self.bids) and rem_qty > 0 and price <= self.bids[i].price:
            if self.bids[i].quantity > rem_qty:
                self.bids[i].quantity -= rem_qty
                self.flip_balance(self.bids[i].user_name, username, rem_qty, self.bids[i].price)
                print(f"Ask Satisfied Successfully at price: {self.bids[i].price} and quantity: {rem_qty} BTC")
                rem_qty = 0
            else:
                rem_qty -= self.bids[i].quantity
                self.flip_balance(self.bids[i].user_name, username, self.bids[i].quantity, self.bids[i].price)
                print(f"Ask Satisfied Partially at price: {self.bids[i].price} and quantity: {self.bids[i].quantity} BTC")
                del self.bids[i]
                continue
            i += 1

        if rem_qty > 0:
            ask = Order(username, "ask", price, rem_qty)
            self.asks.append(ask)
            print(f"Remaining quantity of asks added to Orderbook (Order ID: {ask.order_id})")

        if rem_qty == 0:
            print("Complete Ask Satisfied Successfully")
        return "Ask added successfully."

    def add_market_bid(self, username, quantity):
        rem_qty = quantity
        self.asks.sort(key=lambda x: x.price)

        i = 0
        while i < len(self.asks) and rem_qty > 0:
            if self.asks[i].quantity > rem_qty:
                self.asks[i].quantity -= rem_qty
                self.flip_balance(username, self.asks[i].user_name, rem_qty, self.asks[i].price)
                print(f"Market Bid Satisfied at price: {self.asks[i].price} and quantity: {rem_qty} BTC")
                rem_qty = 0
            else:
                rem_qty -= self.asks[i].quantity
                self.flip_balance(username, self.asks[i].user_name, self.asks[i].quantity, self.asks[i].price)
                print(f"Market Bid Satisfied Partially at price: {self.asks[i].price} and quantity: {self.asks[i].quantity} BTC")
                del self.asks[i]
                continue
            i += 1

        if rem_qty > 0:
            print(f"Insufficient liquidity to fill market bid. Remaining: {rem_qty} BTC")
        else:
            print("Market Bid Filled Successfully")
        return "Market bid processed."

    def add_market_ask(self, username, quantity):
        rem_qty = quantity
        self.bids.sort(key=lambda x: -x.price)

        i = 0
        while i < len(self.bids) and rem_qty > 0:
            if self.bids[i].quantity > rem_qty:
                self.bids[i].quantity -= rem_qty
                self.flip_balance(self.bids[i].user_name, username, rem_qty, self.bids[i].price)
                print(f"Market Ask Satisfied at price: {self.bids[i].price} and quantity: {rem_qty} BTC")
                rem_qty = 0
            else:
                rem_qty -= self.bids[i].quantity
                self.flip_balance(self.bids[i].user_name, username, self.bids[i].quantity, self.bids[i].price)
                print(f"Market Ask Satisfied Partially at price: {self.bids[i].price} and quantity: {self.bids[i].quantity} BTC")
                del self.bids[i]
                continue
            i += 1

        if rem_qty > 0:
            print(f"Insufficient liquidity to fill market ask. Remaining: {rem_qty} BTC")
        else:
            print("Market Ask Filled Successfully")
        return "Market ask processed."

    def get_balance(self, username):
        if username in self.users:
            print("User found")
            print("User balance is as follows:")
            for market, value in self.users[username].user_balance.balance.items():
                print(f"{market} : {value}")
            return "Balance retrieved successfully."
        else:
            print("User not found!!")
            return "User not found"

    def get_quote(self, qty):
        self.asks.sort(key=lambda x: (x.price, x.insertion_order_ask))
        
        remaining = qty
        for ask in self.asks:
            if remaining > 0 and remaining <= ask.quantity:
                print(f"{TICKER} -> Quantity available: {remaining} BTC at {ask.price} USD")
                return "Quote retrieved successfully."
            elif remaining > 0 and remaining > ask.quantity:
                print(f"{TICKER} -> Quantity available: {ask.quantity} BTC at {ask.price} USD")
                remaining -= ask.quantity
        print("Quote retrieved successfully.")
        return "Quote retrieved successfully."

    def get_depth(self):
        self.asks.sort(key=lambda x: x.price, reverse=True)
        self.bids.sort(key=lambda x: x.price, reverse=True)

        print("Order Book\n")
        print(f"{'Price(USDT)':<15} {'Amount(BTC)':<15} {'TOTAL':<15}")
        print("\nASK")
        for ask in self.asks:
            total = ask.price * ask.quantity
            total_str = f"{total:.2f}" if total < 1000 else f"{total/1000:.2f}K"
            print(f"{ask.price:<15.2f} {ask.quantity:<15.5f} {total_str:<15}")

        print("\nBID")
        for bid in self.bids:
            total = bid.price * bid.quantity
            total_str = f"{total:.2f}" if total < 1000 else f"{total/1000:.2f}K"
            print(f"{bid.price:<15.2f} {bid.quantity:<15.5f} {total_str:<15}")
        return "Order book displayed"

    def add_balance(self, username, market, value):
        if username in self.users:
            self.users[username].user_balance.add_balance(market, value)
            print("Balance added successfully")
            return "Balance added successfully"
        print("User not found!! Please enter the right Username to add balance!")
        return "User not found"

    def cancel_ask(self, username, order_id=-1, price=0, quantity=0):
        for i, ask in enumerate(self.asks):
            if order_id != -1 and ask.order_id == order_id and ask.user_name == username:
                del self.asks[i]
                print(f"Ask cancelled successfully (Order ID: {order_id})")
                return
            elif order_id == -1 and ask.user_name == username and ask.price == price:
                if ask.quantity == quantity:
                    del self.asks[i]
                    print("Ask cancelled successfully")
                    return
                elif ask.quantity > quantity:
                    ask.quantity -= quantity
                    print("Ask partially cancelled successfully")
                    return
                else:
                    print("Ask quantity is less than the quantity you want to cancel")
                    return
        print("Ask not found!! Please enter the right Username, Order ID, Price, and Quantity!")

    def cancel_bid(self, username, order_id=-1, price=0, quantity=0):
        for i, bid in enumerate(self.bids):
            if order_id != -1 and bid.order_id == order_id and bid.user_name == username:
                del self.bids[i]
                print(f"Bid cancelled successfully (Order ID: {order_id})")
                return
            elif order_id == -1 and bid.user_name == username and bid.price == price:
                if bid.quantity == quantity:
                    del self.bids[i]
                    print("Bid cancelled successfully")
                    return
                elif bid.quantity > quantity:
                    bid.quantity -= quantity
                    print("Bid partially cancelled successfully")
                    return
                else:
                    print("Bid quantity is less than the quantity you want to cancel")
                    return
        print("Bid not found!! Please enter the right Username, Order ID, Price, and Quantity!")

    def get_trade_history(self):
        if not self.trade_history:
            print("No trades have occurred yet.")
            return "No trades"
        print("Trade History\n")
        print(f"{'Buyer':<15} {'Seller':<15} {'Price':<15} {'Quantity':<15} {'Timestamp'}")
        for trade in self.trade_history:
            print(f"{trade.buyer:<15} {trade.seller:<15} {trade.price:<15.2f} {trade.quantity:<15.5f} {trade.timestamp}")
        return "Trade history displayed"

    def get_spread(self):
        if not self.bids or not self.asks:
            print("Insufficient data to calculate spread.")
            return "No spread available"
        self.bids.sort(key=lambda x: x.price, reverse=True)
        self.asks.sort(key=lambda x: x.price)
        best_bid = self.bids[0].price
        best_ask = self.asks[0].price
        spread = best_ask - best_bid
        print(f"Best Bid: {best_bid} | Best Ask: {best_ask} | Spread: {spread} USD")
        return "Spread calculated"

def main():
    EXCH = OrderBook()
    print(f"\n=========== WELCOME TO THE {TICKER} MARKET AND HAPPY TRADING ===========\n")
    print("\n=========== INITIAL BTC MARKET PRICES ===========\n")
    EXCH.get_depth()

    while True:
        print(f"\n=========== {TICKER} Trading Platform ===========\n")
        print("1. Sign Up User")
        print("2. Add Balance to User Account")
        print("3. Check Current Market Prices")
        print("4. Add Limit Bid")
        print("5. Add Limit Ask (Sell)")
        print("6. Add Market Bid")
        print("7. Add Market Ask (Sell)")
        print("8. Get Current Quote")
        print("9. Check User Balance")
        print("10. Cancel Bid")
        print("11. Cancel Ask")
        print("12. View Trade History")
        print("13. View Bid-Ask Spread")
        print("14. Exit")
        choice = input("Enter your choice: ")

        try:
            choice = int(choice)
            if choice == 1:
                username = input("Enter username: ")
                EXCH.make_user(username)

            elif choice == 2:
                username = input("Enter username: ")
                currency = input("Enter currency (USD/BTC): ")
                amount = float(input("Enter amount: "))
                EXCH.add_balance(username, currency, amount)

            elif choice == 3:
                print("\n=========== CURRENT MARKET PRICES ===========\n")
                EXCH.get_depth()

            elif choice == 4:
                username = input("Enter username: ")
                price = float(input("Enter bid price: "))
                amount = float(input("Enter amount: "))
                EXCH.add_bid(username, price, amount)

            elif choice == 5:
                username = input("Enter username: ")
                price = float(input("Enter ask price: "))
                amount = float(input("Enter amount: "))
                EXCH.add_ask(username, price, amount)

            elif choice == 6:
                username = input("Enter username: ")
                amount = float(input("Enter amount: "))
                EXCH.add_market_bid(username, amount)

            elif choice == 7:
                username = input("Enter username: ")
                amount = float(input("Enter amount: "))
                EXCH.add_market_ask(username, amount)

            elif choice == 8:
                amount = float(input("Enter amount of BTC to quote: "))
                EXCH.get_quote(amount)

            elif choice == 9:
                username = input("Enter username: ")
                EXCH.get_balance(username)

            elif choice == 10:
                username = input("Enter username: ")
                use_id = int(input("Cancel by Order ID? (1 = Yes, 0 = No): "))
                if use_id:
                    order_id = int(input("Enter Order ID: "))
                    EXCH.cancel_bid(username, order_id=order_id)
                else:
                    price = float(input("Enter bid price: "))
                    amount = float(input("Enter amount: "))
                    EXCH.cancel_bid(username, price=price, quantity=amount)

            elif choice == 11:
                username = input("Enter username: ")
                use_id = int(input("Cancel by Order ID? (1 = Yes, 0 = No): "))
                if use_id:
                    order_id = int(input("Enter Order ID: "))
                    EXCH.cancel_ask(username, order_id=order_id)
                else:
                    price = float(input("Enter ask price: "))
                    amount = float(input("Enter amount: "))
                    EXCH.cancel_ask(username, price=price, quantity=amount)

            elif choice == 12:
                EXCH.get_trade_history()

            elif choice == 13:
                EXCH.get_spread()

            elif choice == 14:
                print("\nThank you for trading. Goodbye!")
                sys.exit(0)

            else:
                print("Invalid choice. Please try again.")

        except ValueError:
            print("Please enter valid numerical input where required.")
        except Exception as e:
            print(f"An error occurred: {e}")

if __name__ == "__main__":
    main()