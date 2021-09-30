/* To compile, run:
    g++ interrupt.cpp -lpigpio -lrt -lpthread -o interrupt 
*/
#include <iostream>
#include <thread>
#include <mutex>

extern "C" {
    #include <unistd.h> // sleep
}

bool signal_arrived = false;
int counter = 0;
std::mutex counter_mutex;
std::mutex signal_mutex;

void thread_callback(bool* signal_arrived, int* counter){
    char reading;
    while (1){
        std::lock_guard<std::mutex> counter_guard(counter_mutex);
        std::lock_guard<std::mutex> signal_guard(signal_mutex);
        std::cout << "Waiting for reading:\n";
        std::cin >> reading;  
        if (reading == 'c'){
            *signal_arrived = true;
            *counter = *counter +1;
        }    
    }
}

int main(int argc, char const *argv[]){
    std::thread sensor_reading(thread_callback, &signal_arrived, &counter);
    while (1){
        std::cout << "main thread running\n";
        if (signal_arrived == true){
            std::cout << "signal arrived, " << "counter=" << counter << "\n";
            signal_arrived = false;
        }
        usleep(1000*1000);
    }
    
    return 0;
}
