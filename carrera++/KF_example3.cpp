/*  This program takes real sensor data and filter it
    Run this programm on Pi which is connected to MPU
    To compile, run
        g++ KF_example3.cpp KF.cpp mpu6050.cpp -o ../build/KF_example3 -I /usr/include/eigen3 -Wno-psabi -li2c

*/
#include <iostream>
#include <cstdlib> //exit
#include <ctime>
#include "KF.h"
#include "mpu6050.h"
#include "dashboard.h"
extern "C" {
    #include <unistd.h>     // sleep
}
#define CALIBRATION_ACC_TG 2.55 // off-set [m/s^2]of acc_tg due to gravity
#define GRAVITY_STG 9.80884              // gravitation in Stuttgart [N/kg]

// save accel, velo, etc in Dashboard
Dashboard dsb;

// sampling periode [s]
const double T_sample = 0.1;    

// Kalman Filter
KF kf(3,3,1); // dim_x, dim_z, dim_u

// saves prior/posterior state estimate
Vector state_tg(3);

// saves measurements
Vector measm(3);

// set matrix print-layout
Eigen::IOFormat fmt(4, 0, "\t", "\n", "\t[", "]"); 


int main(){
    // sampling periode 
    const double T_sample = 1.0;  

    // state transistion mtx 
    Matrix A(3,3);
    A << 1.0, T_sample, T_sample*T_sample/2,
         0.0,      1.0,            T_sample, 
         0.0,      0.0,                 0.0;

    // control input mtx
    Matrix B(3,1);
    B << 0.0,
         0.0,
         1.0;
         
    // measurement output mtx
    Matrix C(3,3);
    C << 1.0, 0.0, 0.0,
         0.0, 1.0, 0.0,
         0.0, 0.0, 1.0;

    // model noise cov
    Matrix Q(3,3);
    Q << 1.0, 0.0, 0.0,
         0.0, 1.0, 0.0,
         0.0, 0.0, 1.0;

    // measurement noise cov
    Matrix R(3,3);
    R << 1.0, 0.0, 0.0,
         0.0, 1.0, 0.0,
         0.0, 0.0, 1.0;
    
    // control input: accel in z-direction
    Matrix Acc_tg(1,1);
    Acc_tg << 0;
    
    // set up Kalman Filter
    kf.set_up_model(A, B, C);    
    kf.set_model_noise(Q);
    kf.set_measure_noise(R);

    ////////////////////////////// set up MPU6050 //////////////////////////////
    MPU6050 mpu;
    if(!mpu.init()){
        std::cout << "Failed to initiate MPU6050\n";
        exit(1);
    }
    mpu.config();
    ///////////////////////////////// Big while-loop /////////////////////////////////
    while (1){
        // read accelerations
        mpu.read_acc(&(dsb.acc_x), &(dsb.acc_y), &(dsb.acc_z));
        std::cout << "read data: " << dsb.acc_x << "\t" << dsb.acc_y << "\t" << dsb.acc_z << "\n";
        

        // convert acceleration to N/kg
        dsb.acc_x *= -GRAVITY_STG;
        dsb.acc_y *= -GRAVITY_STG;
        dsb.acc_z *= -GRAVITY_STG;
        
        // calibrate tagential accel
        dsb.acc_z -= CALIBRATION_ACC_TG;
        
        // perform kalman filtering
        Acc_tg << dsb.acc_z;
        kf.predict(Acc_tg);
        kf.update();
        
        // save prior state estimate in vector x
        state_tg << kf.get_prio_state_estm(); 
        std::cout << "state estimate=\n" << state_tg.format(fmt) << std::endl;
        usleep(T_sample*1000*1000);
    }
    
    return 0;
}

