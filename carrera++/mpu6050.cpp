#include "mpu6050.h"

void config_mpu(int i2c_fd){
    i2c_smbus_write_byte_data(i2c_fd, SMPLRT_DIV,   7);
    i2c_smbus_write_byte_data(i2c_fd, PWR_MGMT_1,   1);
    i2c_smbus_write_byte_data(i2c_fd, CONFIG,       0);
    i2c_smbus_write_byte_data(i2c_fd, ACCEL_CONFIG, 0);
    i2c_smbus_write_byte_data(i2c_fd, INT_ENABLE,   1);
}

int read_reg(int i2c_fd, int reg){
    int high = i2c_smbus_read_byte_data(i2c_fd, reg);
    int low  = i2c_smbus_read_byte_data(i2c_fd, reg+1);

    int value = (high<<8)|low;
    if (value > 32768){
        value = value -65536;
    }
    return value;
}

void read_acc(int i2c_fd, double *Ax, double *Ay, double *Az){
    int Ax_raw = read_reg(i2c_fd, ACCEL_XOUT_H);
    int Ay_raw = read_reg(i2c_fd, ACCEL_YOUT_H);
    int Az_raw = read_reg(i2c_fd, ACCEL_ZOUT_H);

    *Ax = (Ax_raw/16384.0);
    *Ay = (Ay_raw/16384.0);
    *Az = (Az_raw/16384.0);    
}
