/*  Before you complile, install the library:
        sudo apt-get install libi2c-dev
    To compile, run:
        g++ mpu_driver.cpp mpu6050.cpp -li2c 
*/

// .cpp starts here
#include <iostream>
#include "mpu6050.h"
#include <cstdio>

int main(int argc, char const *argv[]){
    // open the i2c bus
    int i2c_fd;
    int adapter_nr = 1; // inspect /sys/class/i2c-dev/ or run "i2cdetect -l"
    char i2c_path[20];
    std::snprintf(i2c_path, 19, "/dev/i2c-%d", adapter_nr);

    i2c_fd = open(i2c_path, O_RDWR);
    if (i2c_fd < 0) {
        std::cout << "Can not open " << i2c_path << "\n";
        exit(1);
    }

    // link the i2c file descriptor to MPU6050
    if (ioctl(i2c_fd, I2C_SLAVE, MPU6050_ADDR) < 0) {
        std::cout << "Unable to link to " << MPU6050_ADDR << "\n";
        exit(1);
    }

    // configure registers in MPU6050 
    config_mpu(i2c_fd);
    // sensor read out
    double Ax, Ay, Az;

    // read data
    while (1){
        read_acc(i2c_fd, &Ax, &Ay, &Az);
        std::cout << "Ax=" << Ax << "g\t"
                  << "Ay=" << Ay << "g\t"
                  << "Az=" << Az << "g\n";
        usleep(1*1000*1000);
    } 
    return 0;
}




