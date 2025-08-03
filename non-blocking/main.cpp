#include <thread>
#include <cassert>
#include <vector>

int main(){

    int num_iterations = 1<<15;
    int num_threads = 8;

    std::atomic<int> sink=0;

    auto work = [&](){
        while(true){
            int expected = sink.load();
            int desired;

            do{
                if(expected==num_iterations) return;

                desired=expected+1;
            }while(!sink.compare_exchange_strong(expected,desired));
        }
    };

    auto slow_work = [&](){
        while(true){
            int expected = sink.load();
            int desired;

            do{
                if(expected==num_iterations) return;

                desired=expected+1;
            }while(!sink.compare_exchange_strong(expected,desired));
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    };

    std::vector<std::thread> threads;
    for(int i=0;i<num_threads-1;i++){
        threads.emplace_back(work);
    }
    threads.emplace_back(slow_work);

    for(auto &th: threads) th.join();

    return 0;
}
