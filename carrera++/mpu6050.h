#ifndef MPU6050_H
#define MPU6050_H

// Registers of MPU6050
#define PWR_MGMT_1    0x6B
#define SMPLRT_DIV    0x19
#define CONFIG        0x1A
#define ACCEL_CONFIG  0x1C
#define INT_ENABLE    0x38
#define ACCEL_XOUT_H  0x3B
#define ACCEL_YOUT_H  0x3D 
#define ACCEL_ZOUT_H  0x3F

// Address of MPU6050
#define MPU6050_ADDR  0x68

extern "C"{
    #include <linux/i2c-dev.h>
    #include <i2c/smbus.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <fcntl.h>
}

// Functions
// int init_i2c_bus(int adapter_nr, int device_addr);

// Configure MPU6050: power, precesions, ...
void config_mpu(int i2c_fd);

// read acceleration data and save into Ax, Ay, Az
void read_acc(int i2c_fd, double *Ax, double *Ay, double *Az);

// read register 'reg' and return raw value
short read_reg(int i2c_fd, int reg);

#endif


