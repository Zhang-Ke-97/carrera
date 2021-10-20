/* To compile, run:
    g++-11 -I /usr/local/include/eigen3 KF.cpp KF2_driver.cpp -o ../build/KF2_driver
*/

/* Test file: 1D movement with const velocity
    state vector x = [position, velocity]^T
    measurement  z = position
*/
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "KF.h"
#include <iostream>

#include <random> // for 2-dim KF testing
#include <cmath> // for 2-dim KF testing

// read the current mileage
static double read_position(int t_step); 

// velocity of the object
double velocity = 5;

// sampling periode 
const double T_sample = 0.1;

// set matrix print-layout
Eigen::IOFormat fmt(4, 0, "\t", "\n", "\t[", "]"); 

int main(){
    // set up matrices
    Matrix A(2,2);
    A << 1.0, T_sample,
         0.0,  1.0; 

    Matrix B(2,1);

    B << 0.0, 
         0.0;
    Matrix C(1,2);

    C << 1.0, 0.0;
    
    // define kalman filter
    KF kf = KF(2,1,1);
    kf.set_up_model(A, B, C);
    
    // show info
    std::cout << "Kalman Filter tracking an object with constant velocity of " << velocity << "\n";
    std::cout << "initial estimate:\n" << kf.get_post_state_estm().format(fmt) << "\n";
    std::cout << "initial error covariance:\n" << kf.get_error_covariance().format(fmt) << "\n\n";
    kf.info();
    
    // start filtering
    for (int i = 0; i < 21; i++){   
        kf.predict();
        Vector measurement {{read_position(i)}};
        kf.update(measurement);
        
        std::cout << "t=" << i*T_sample << "\n";
        std::cout << "measured:\n" << measurement.format(fmt) << "\n";
        std::cout << "estimate:\n" << kf.get_post_state_estm().format(fmt) << "\n";
        std::cout << "error variance:\n" << kf.get_error_covariance().format(fmt) << "\n\n";
        sleep(1);
    }   
    return 0;
}


static double read_position(int t_step){
    double position_init = 10;
    // set up gaussian noise generator
    double mean = 0.0;
    double stddev = 0.1;
    std::default_random_engine generator;
    std::normal_distribution<double> dist(mean, stddev);
    return position_init + velocity*t_step*T_sample + dist(generator);
}