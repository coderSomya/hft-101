#include <iostream>
#include <thread>
#include <deque>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <sys/time.h>

#define LOG(X) { std::cout << X << std::endl; }

const int MESSAGES_COUNT = 1000;

struct Order{
    uint64_t timestamp;
    int seqno;
    double price;
    int qty;
};

uint64_t timestamp(){
    timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

std::deque<Order> msg_queue;
std::mutex mutex;
std::condition_variable cv;


void producer_thread_entry(){
    LOG("Producer thread started");


    for(int i=0; i<MESSAGES_COUNT; i++){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        Order order;
        
        order.timestamp = timestamp();
        order.seqno = i;
        order.price = 100.0 + i;
        order.qty = 100 + i;

        {
            std::unique_lock<std::mutex> lock(mutex);
            msg_queue.push_back(order);
            cv.notify_one();
        }
        
    }

    LOG("Producer thread finished");
}

void consumer_thread_entry(){
    LOG("Consumer thread started");

    int count_recieved = 0;

    while(count_recieved < MESSAGES_COUNT){

        std::unique_lock<std::mutex> lock(mutex);

        //busy loop
        if(msg_queue.empty()){
            continue;
        }
        //cv.wait(lock, [](){return !msg_queue.empty();});
        
        auto &order = msg_queue.front();
        msg_queue.pop_front();
        count_recieved++;

        uint64_t time_recieved = timestamp();
        uint64_t time_diff = time_recieved - order.timestamp;

        LOG("Recieved order: " + std::to_string(order.seqno) + " with latency " + std::to_string(time_diff));

        LOG("Consumer thread received order: " + std::to_string(order.seqno));
    }
}



int main(){

    std::thread producer {producer_thread_entry};
    std::thread consumer {consumer_thread_entry};

    // Wait for threads to complete
    producer.join();
    consumer.join();
    
    return 0;
}