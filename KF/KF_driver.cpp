/* To compile, run:
    g++-11 -I /usr/local/include/eigen3 KF.cpp KF_driver.cpp -o ../build/KF_driver
*/
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "KF.h"
#include <iostream>

#include <random> // for 1-dim KF testing
#include <cmath> // for 1-dim KF testing

int main(){
    Matrix A {{2.0}};
    Matrix B {{0.0}};
    Matrix C {{1.0}};
    Matrix Q {{2.0}}; // model noise
    Matrix R {{1.0}}; // measurement noise
    Matrix P {{1000.0}}; // initial error cov
    Vector x {{0.0}}; // initial state estimate
    Vector u {{0.0}}; 
    Vector z {{0.0}};

    KF kf = KF(A, B, C, Q, R, P, x, u, z);
    double mean = 0.0;
    double stddev = 0.1;
    std::default_random_engine generator;
    std::normal_distribution<double> dist(mean, stddev);
    
    std::cout << "start kalman filering...\n";
    std::cout << "estimate:" << kf.get_state_estimate() << "\n";
    std::cout << "error variance:" << kf.get_error_covariance() << "\n\n";
    
    for (int i = 0; i < 11; i++){   
        kf.predict();
        Vector measurement {{std::pow(2,i)+dist(generator)}};
        kf.update(measurement);
        
        std::cout << "measured: " << measurement << "\n";
        std::cout << "estimate:" << kf.get_state_estimate() << "\n";
        std::cout << "error variance:" << kf.get_error_covariance() << "\n\n";
        sleep(1);
    }
    
    return 0;
}


int main2()
{
    double T = 0.1; // sampling periode
    // set state transition matrix
    Matrix A {
        {1.0,  T,   T*T/2},
        {0.0,  1.0, T},
        {0.0,  0.0, 0.0}
    };
    // set the control matrix
    Matrix B {
        {0.0},
        {0.0},
        {1.0}
    };
    // set the measurement matrix
    Matrix C {
        {1.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}  
    };
    // set the covariance of model noise
    Matrix Q {
        {1.0,  0.0,  0.0},
        {0.0,  1.0,  0.0},
        {0.0,  0.0,  1.0}
    };
    // set the covariance of measurement noise
    Matrix R {
        {0.5, 0.0},
        {0.0, 0.5}
    };
    // set the initial error covariance matrix
    Matrix P {
        {1000.0,   0.0,      0.0},
        {0.0,      1000.0,   0.0},
        {0.0,      0.0,      1000.0},
    };
    // set the initial state estimate
    Vector x{
        {0},
        {0},
        {0},
    };
    // set the initial measurements
    Vector z {
        {0},
        {0}
    };
    // set the initial input
    Vector u {
        {0}
    };

    KF kf = KF(A, B, C, Q, R, P, x, z, u);
    
    return 0;
}
