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

// Functions
int init_i2c_bus(int adapter_nr, int device_addr);
void config_mpu(int i2c_fd);
void read_acc(int i2c_fd, double *Ax, double *Ay, double *Az);
int read_reg(int i2c_fd, int reg);

extern "C"{
    #include <linux/i2c-dev.h>
    #include <i2c/smbus.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <fcntl.h>
}

#include <cmath>
// .cpp starts here
#include <iostream>

int main(int argc, char const *argv[]){
    // open the i2c bus
    int i2c_fd;
    int adapter_nr = 1; // inspect /sys/class/i2c-dev/ or run "i2cdetect -l"
    char i2c_path[20];
    snprintf(i2c_path, 19, "/dev/i2c-%d", adapter_nr);

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

void config_mpu(int i2c_fd){
    i2c_smbus_write_byte_data(i2c_fd, SMPLRT_DIV,   7);
    i2c_smbus_write_byte_data(i2c_fd, PWR_MGMT_1,   1);
    i2c_smbus_write_byte_data(i2c_fd, CONFIG,       0);
    i2c_smbus_write_byte_data(i2c_fd, ACCEL_CONFIG, 0);
    i2c_smbus_write_byte_data(i2c_fd, INT_ENABLE,   1);
}

int read_reg(int i2c_fd, int reg){
    int high = i2c_smbus_read_byte_data(i2c_fd, ACCEL_XOUT_H);
    int low  = i2c_smbus_read_byte_data(i2c_fd, ACCEL_XOUT_H+1);

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

    *Ax = std::round(Ax_raw/16384.0);
    *Ay = std::round(Ay_raw/16384.0);
    *Az = std::round(Az_raw/16384.0);    
}


