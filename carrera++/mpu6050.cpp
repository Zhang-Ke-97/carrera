#include "mpu6050.h"
#include <iostream>

MPU6050::MPU6050(int adp /* =1 */){
    adapter_nr = adp;
}

MPU6050::~MPU6050(){}

bool MPU6050::init(){
    // open i2c bus
    char i2c_path[20];
    std::snprintf(i2c_path, 19, "/dev/i2c-%d", adapter_nr);
    i2c_fd = open(i2c_path, O_RDWR);
    
    if (i2c_fd < 0) {
        std::cout << "Can not open " << i2c_path << "\n";
        return false;
    }

    // link the i2c file descriptor to MPU6050
    if (ioctl(i2c_fd, I2C_SLAVE, MPU6050_ADDR) < 0) {
        std::cout << "Unable to link to " << MPU6050_ADDR << "\n";
        return false;
    }
    return true;
}

void MPU6050::config(){
    i2c_smbus_write_byte_data(i2c_fd, SMPLRT_DIV,   7);
    i2c_smbus_write_byte_data(i2c_fd, PWR_MGMT_1,   1);
    i2c_smbus_write_byte_data(i2c_fd, CONFIG,       0);
    i2c_smbus_write_byte_data(i2c_fd, ACCEL_CONFIG, 0);
    i2c_smbus_write_byte_data(i2c_fd, INT_ENABLE,   1);    
}

short MPU6050::read_reg(int reg){
    short high = i2c_smbus_read_byte_data(i2c_fd, reg);
    short low  = i2c_smbus_read_byte_data(i2c_fd, reg+1);

    short value = (high<<8)|low;
    if (value > 32768){
        value = value -65536;
    }
    return value;
}


void MPU6050::read_acc(double *Ax, double *Ay, double *Az){
    short Ax_raw = read_reg(ACCEL_XOUT_H);
    short Ay_raw = read_reg(ACCEL_YOUT_H);
    short Az_raw = read_reg(ACCEL_ZOUT_H);

    *Ax = (Ax_raw/16384.0);
    *Ay = (Ay_raw/16384.0);
    *Az = (Az_raw/16384.0);    
}
