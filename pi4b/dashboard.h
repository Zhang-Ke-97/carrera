/**
 * @file dashboard.h
 * @brief Header file for the struct Dashboard 
 * @author Ke Zhang
 * @date 31. October 2021
 */
#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <ctime>

/**
 * @struct Dashboard
 * @brief Stores the measured physical quantities of the car.
 * 
 */ 
struct Dashboard{
    int cycle = -1; /** number of cycles the car has travled so far */

    double mileage = 0.0; /** mileage of the car */   

    double acc_x = 0.0; /** acceleration in x direction received from Pi0 */
    double acc_y = 0.0; /** acceleration in y direction received from Pi0 */
    double acc_z = 0.0; /** acceleration in z direction received from Pi0 */   

    clock_t t_gate1 = 0; /** time when the car passes gate1/gate2 */
    clock_t t_gate2 = 0; /** time when the car passes gate1/gate2 */  

    double velo = 0.0; /** velocity of the car computed from gate 1 and 2 */

    /**
     * @brief computes the velocity of the car
     * 
     * The velocity is computed from the gate distance and 
     * the time which the car spent on passing the gates
     * @param gate_distance distance between two gates
     * @return none
     */ 
    void compute_velocity(double gate_distance){
        double duration_in_sec = double(t_gate2-t_gate1)/CLOCKS_PER_SEC;
        velo = gate_distance/duration_in_sec;
    }

    // computes the mileage from cycle and bahn length
    /**
     * @brief computes the mileage covered so far
     * @param carrera_bahn_length The full length of the Carrera track
     * @return none
     */ 
    void compute_mileage(double carrera_bahn_length){
        mileage = cycle*carrera_bahn_length;
    }

    /**
     * @brief record the time when the car passing by gate 1
     * @return none
     */ 
    void set_t_gate1(){
        t_gate1 = clock();
    }
   /**
     * @brief record the time when the car passing by gate 2
     * @return none
     */ 
    void set_t_gate2(){
        t_gate2 = clock();
    }

};

#endif
