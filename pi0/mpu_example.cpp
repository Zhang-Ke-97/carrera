/*  Before you complile, install the library:
        sudo apt-get install libi2c-dev
    To compile, run:
        g++ mpu_driver.cpp mpu6050.cpp -li2c 
*/

#include <iostream>
#include <cstdlib>
extern "C" {
    #include <unistd.h>     // sleep
}
#include "mpu6050.h"


int main(int argc, char const *argv[]){
    // number of iterations for calibration
    int itr_calb = 0;
    std::cout << "How many iterations do you want for calibration? ";
    std::cin >> itr_calb;

    // create mpu object
    MPU6050 mpu;

    // initialise mpu unit (i2c bus, file discriptor etc...)
    mpu.init();

    // configure registers in mpu
    mpu.config();

    // calibrate mpu unit
    mpu.calibrate(200);

    // storage of sensor read out
    double Ax, Ay, Az;

    // read calibrated data
    while (1){
        mpu.read_acc(&Ax, &Ay, &Az, 1);
        std::cout << "Ax= " << Ax << "g, "
                  << "Ay= " << Ay << "g, "
                  << "Az= " << Az << "g\n";
        usleep(1*1000*1000);
    } 
    return 0;
}



