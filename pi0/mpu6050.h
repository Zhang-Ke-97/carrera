/**
 * @file mpu6050.h
 * @brief Header file for the class MPU6050 
 * @author Ke Zhang
 * @date 31. October 2021
 * The address of registers of MPU6050 are defined as macros.
 * The set-ups of i2C bus and the i2C system calls are incorperated in member functions. 
 * The full data sheet of MPU6050 can be found at https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf
 */

#ifndef MPU6050_H
#define MPU6050_H

#define PWR_MGMT_1    0x6B /** @brief power management  */
#define SMPLRT_DIV    0x19 /** @def sample rate divider */
#define CONFIG        0x1A /** @def configeration */
#define ACCEL_CONFIG  0x1C /** @def accelerometer */
#define INT_ENABLE    0x38 /** @def interrupt enable */
#define ACCEL_XOUT_H  0x3B /** @def accerlaration read-out in x direction */
#define ACCEL_YOUT_H  0x3D /** @def accerlaration read-out in y direction */
#define ACCEL_ZOUT_H  0x3F /** @def accerlaration read-out in z direction */

#define MPU6050_ADDR  0x68 /** Address of MPU6050 */

extern "C"{
    #include <linux/i2c-dev.h>
    #include <i2c/smbus.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <fcntl.h>
}

/**
 * @class MPU6050
 * @brief User-friendly interface to program the MPU6050 unit
 * 
 * The set-ups of i2C bus and the i2C system calls are incorperated in member functions. 
 */ 
class MPU6050{
private:
    int i2c_fd = MPU6050_ADDR; /** file disciptor to access i2c bus */
    int adapter_nr; /** adatper number, set to 1 by default */

    double Ax_mean; /** stores the mean value of acceleration in x direction (for calibration) */
    double Ay_mean; /** stores the mean value of acceleration in y direction (for calibration) */
    double Az_mean; /** stores the mean value of acceleration in z direction (for calibration) */

public:
    /**
     * @brief Default constructor
     * @param adp dapter number (default 1)
     */ 
    MPU6050(int adp = 1);

    /**
     * @brief Default destructor
     */ 
    ~MPU6050();
    
    // set up i2c bus and link it to MPU6050
    // returns true if successful, false otherwise
    /**
     * @brief Initialise MPU6050
     * @return true if successful, false otherwise.
     * This function opens i2c bus and links i2c file descriptor to MPU6050
     */
    bool init();
    
    /**
     * @brief Configure MPU6050
     * @return none
     * This function configures the registers of MPU6050 with default setups.
     * By default, the sample rate divider is configured so that gyroscope output rate = 8kHz.
     * Power management register is configured to sleep mode.
     * Configure register is set to 1.
     * Accelerometer is configred to the full scale range from -2g to 2g.
     * Interrupt is enabled by default.
     */ 
    void config();
    
    // 
    /**
     * @brief Read register and return raw value
     * @param reg The address of the reg to be read
     * @return Raw value in the register reg
     */ 
    short read_reg(int reg);

    // compute the mean of acceleration data
    /**
     * @brief Calibrate the MPU6050
     * @param samples Number of samples used to calibrate MPU6050
     * @return none
     * This function computes the mean value of n samples of acceleration data, 
     * where n is specified by the parameter samples. 
     * The acceleration data are centered to 0 with a subtraction by its mean.
     */ 
    void calibrate(int samples=100);
    
    // read acceleration data and save into Ax, Ay, Az, 
    // option: calibration or not
    /**
     * @brief Read acceleration data
     * @param Ax Pointer to the variable storing the acceleration in x direction
     * @param Ay Pointer to the variable storing the acceleration in y direction
     * @param Az Pointer to the variable storing the acceleration in z direction
     * @param calb Flag for whether calibration will be performed or not.
     * @return none
     */
    void read_acc(double* Ax, double *Ay, double *Az, bool calb=false);
};


#endif


