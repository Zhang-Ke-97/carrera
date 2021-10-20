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

class MPU6050{
private:
    int i2c_fd = MPU6050_ADDR;
    int adapter_nr;

    double Ax_mean;
    double Ay_mean;
    double Az_mean;

public:
    MPU6050(int adp = 1);
    ~MPU6050();
    
    // set up i2c bus and link it to MPU6050
    // returns true if successful, false otherwise
    bool init();
    
    // Configure MPU6050: power, precesions, ...
    void config();
    
    // read register 'reg' and return raw value
    short read_reg(int reg);

    // compute the mean of acceleration data
    void calibrate(int samples=100);
    
    // read acceleration data and save into Ax, Ay, Az, 
    // option: calibration or not
    void read_acc(double* Ax, double *Ay, double *Az, bool calb=false);
};


#endif


