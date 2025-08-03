#include <thread>
#include <vector>
#include <pthread.h>
#include <mutex>
#include <random>
#include <algorithm>
#include <chrono>
#include <list>

int main(){

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution dist(10,20);

    std::list<int> l;
    std::generate_n(std::back_inserter(l), 1<<20, [&](){
        return dist(mt);
    });

    std::mutex m;

    auto work = [&](){
        while(true){
            std::lock_guard<std::mutex> lg(m);

            if(l.empty()) break;

            l.pop_back();
        }
    };

    int num_threads=8;
    std::vector<std::thread> threads;

    for (int i=0; i<num_threads; i++){
        threads.emplace_back(work);
    }

    for (int i=0; i<num_threads; i++){
        threads[i].join();
    }
    return 0;
}
