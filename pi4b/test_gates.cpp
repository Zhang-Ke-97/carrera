
/** 
 * @file test_gates.cpp
 * @brief Test file for Lichtschranke
 * @author Ke Zhang
 * @date 31. Oktober 2021
 * 
 * Run this program to test whether the interrupt service routines work with your Lichtschranke. 
 * To compile, run
 * g++ test_gates.cpp -o ../build/test_gates -lpthread -lpigpio -lrt
*/
#include <iostream>
#include <cstdlib> //exit
#include <ctime>
extern "C" {
    #include <sys/types.h>
    #include <unistd.h>     // sleep
    #include <pigpio.h>     // GPIO Lib
}
#include "dashboard.h"

#define GPIO_GATE_1 14
#define GPIO_GATE_2 15


clock_t t1 = 0;
clock_t t2 = 0; 
 
/* ISR call back function associated gate 1 */
void gate1_ISR_callback(int gpio, int level, uint32_t tick);

/* ISR call back function associated gate 2 */
void gate2_ISR_callback(int gpio, int level, uint32_t tick);


int main(){
	t1 = clock();
	t2 = clock();

    ///////////////////////////////// set up GPIO /////////////////////////////////
    // Light sensor: output = 1 when dark  <=> car arrived
    //                      = 0 when light <=> no car
    // GPIO input: PULL_DOWN required
    if (gpioInitialise()<0){
        std::cout << "Failed to initialise GPIO\n";
        exit(1);
    }
    gpioSetMode(GPIO_GATE_1, PI_INPUT);
    gpioSetMode(GPIO_GATE_2, PI_INPUT);

    gpioSetPullUpDown(GPIO_GATE_1, PI_PUD_DOWN); 
    gpioSetPullUpDown(GPIO_GATE_2, PI_PUD_DOWN); 

    gpioSetISRFunc(GPIO_GATE_1, RISING_EDGE, -1, gate1_ISR_callback);
    gpioSetISRFunc(GPIO_GATE_1, RISING_EDGE, -1, gate1_ISR_callback);
    gpioSetISRFunc(GPIO_GATE_2, RISING_EDGE, -1, gate2_ISR_callback);


    ///////////////////////////////// Big while-loop /////////////////////////////////
    while (1){
        usleep(1000*1000);
    }
    
    return 0;
}


// ISR call back function associated gate 1
void gate1_ISR_callback(int gpio, int level, uint32_t tick){
	if(clock()-t1 > CLOCKS_PER_SEC/10){
		std::cout << "GATE 1: " << gpio << ", tick= " << tick << std::endl;
	}
	t1 = clock();
}

// ISR call back function associated gate 2
void gate2_ISR_callback(int gpio, int level, uint32_t tick){
	if(clock()-t2 > CLOCKS_PER_SEC/10){
		std::cout << "GATE 2: " << gpio << ", tick= " << tick << std::endl;
	}
	t2 = clock();
}
