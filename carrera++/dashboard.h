#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <ctime>

struct Dashboard{
    // number of cycles the car has travled so far
    int cycle = 0;

    // mileage of the car
    double mileage = 0.0;
    
    // acceleration data received from Pi0
    double acc_x = 0.0;
    double acc_y = 0.0;
    double acc_z = 0.0;
    
    // time when the car passes gate1/gate2
    clock_t t_gate1 = 0;
    clock_t t_gate2 = 0;
    
    // velocity of the car computed from gate 1 and 2
    double velo = 0.0;

    // returns velocity (m/s) of the car at the 2nd laser gate
    // parameter: distance between two gates
    double get_velocity(double gate_distance){
        double duration_in_sec = double(t_gate2-t_gate1)/CLOCKS_PER_SEC;
        velo = gate_distance/duration_in_sec;
        return velo;
    }

    // computes the mileage from cycle and bahn length
    double get_mileage(double carrera_bahn_length){
        mileage = cycle*carrera_bahn_length;
        return mileage;
    }

    void set_t_gate1(){
        t_gate1 = clock();
    }

    void set_t_gate2(){
        t_gate2 = clock();
    }

};

#endif
