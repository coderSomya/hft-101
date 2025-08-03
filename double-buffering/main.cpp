#include <iostream>
#include <random>
#include <span>
#include <thread>
#include <random>
#include <semaphore>


void generate_data(std::span<int> data){

    std::random_device rd;
    std::mt19937 mt(rd());

    std::uniform_int_distribution dist(1,100);

    for(auto &value: data){
        value = dist(mt);
    }
}

void process_data(std::span<int> data){

    for(int i=0; i<5; i++){
        for (auto &value: data){
            value%=value+1;
        }
    }
}


void baseline(){
    // simply calls generate and process funcs
    // sequentially
    std::vector<int> data;
    data.resize(1<<18);

    for(int i=0;i<100;i++){
        generate_data(data);
        process_data(data);
    }
}

void double_buffering(){
    std::vector<int> data_1;
    std::vector<int> data_2;

    data_1.resize(1<<18);
    data_2.resize(1<<18);

    std::binary_semaphore signal_to_process{0};
    std::binary_semaphore signal_to_generate{1};

    const int num_iterations = 100;

    auto data_generator = [&](){
        for (int i=0; i<num_iterations; i++){
            generate_data(data_1);

            signal_to_generate.acquire();

            data_1.swap(data_2);

            signal_to_process.release();
        }
    };

    auto data_processor = [&](){
        for (int i=0; i<num_iterations; i++){
            signal_to_process.acquire();

            process_data(data_2);

            signal_to_generate.release();
        }
    };

    std::thread generator{data_generator};
    std::thread processor{data_processor};

    generator.join();
    processor.join();
}

int main(){

    double_buffering();
    return 0;
}
