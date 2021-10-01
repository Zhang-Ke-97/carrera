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

int main(){
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
