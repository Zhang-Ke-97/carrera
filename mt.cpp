/* To compile, run:
    g++ main.cpp -o main -lpigpio -lrt -lpthread
*/
#include <iostream>
#include <cstdlib> 
extern "C" {
    #include <unistd.h>
    #include <pigpio.h>
}

#include <thread>
#include <mutex>

#define GPIO_GATE_1 14

bool signal_arrived = false;
int counter = 0;
std::mutex counter_mutex;
std::mutex signal_mutex;

void thread_callback(bool* signal_arrived, int* counter){
    // set up GPIO
    if (gpioInitialise()<0){
        std::cout << "Failed to initialise GPIO\n";
        exit(1);
    }
    gpioSetMode(GPIO_GATE_1, PI_INPUT);
    gpioSetPullUpDown(GPIO_GATE_1, PI_PUD_UP);
    
    // Keep reading
    while (1){
        std::lock_guard<std::mutex> counter_guard(counter_mutex);
        std::lock_guard<std::mutex> signal_guard(signal_mutex);
        if (gpioRead(GPIO_GATE_1) == 0){
            *signal_arrived = true;
            *counter = *counter +1;
            usleep(2*1000*1000);
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
