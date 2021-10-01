/* To compile, run:
    g++-11 -I /usr/local/include/eigen3 KF.cpp KF_driver.cpp -o ../build/KF_driver
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

// sampling periode 
const double T_sample = 0.1;

int main(){
    Matrix A {
        {1.0, T_sample},
        {0.0, 1.0}
    };
    Matrix B {
        {0.0},
        {0.0}
    };
    Matrix C {
        {1.0, 0.0}
    };
   // model noise cov
    Matrix Q {
        {1.0, 0.0},
        {0.0, 1.0}
    }; 
    // measurement noise cov
    Matrix R {
        {0.01}
    }; 
    // initial error cov
    Matrix P {
        {1000.0,    0.0},
        {0.0,    1000.0}
    }; 
    // initial state estimate
    Vector x {
        {0.0},
        {0.0}
    }; 
    Vector u {
        {0.0}
    }; 
    Vector z {
        {0.0}
    };

    KF kf = KF(A, B, C, Q, R, P, x, u, z);
    
    // set matrix print-layout
    Eigen::IOFormat fmt(4, 0, "\t", "\n", "\t[", "]"); 
    
    std::cout << "start kalman filering...\n";
    std::cout << "estimate:\n" << kf.get_state_estimate().format(fmt) << "\n";
    std::cout << "error variance:\n" << kf.get_error_covariance().format(fmt) << "\n\n";
    
    for (int i = 0; i < 21; i++){   
        kf.predict();
        Vector measurement {{read_position(i)}};
        kf.update(measurement);
        
        std::cout << "t=" << i*T_sample << "\n";
        std::cout << "measured:\n" << measurement.format(fmt) << "\n";
        std::cout << "estimate:\n" << kf.get_state_estimate().format(fmt) << "\n";
        std::cout << "error variance:\n" << kf.get_error_covariance().format(fmt) << "\n\n";
        sleep(1);
    }   
    return 0;
}


static double read_position(int t_step){
    double velocity = 5;
    double position_init = 10;
    // set up gaussian noise generator
    double mean = 0.0;
    double stddev = 0.1;
    std::default_random_engine generator;
    std::normal_distribution<double> dist(mean, stddev);
    return position_init + velocity*t_step*T_sample + dist(generator);
}