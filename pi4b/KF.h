/** 
 * @file KF.h
 * @brief Header file for the class Kalman Filter (KF)
 * @author Ke Zhang
 * @date 31. October 2021
 * This file implements Kalman filter based on the library Eigen.
 */
#ifndef KF_H
#define KF_H

#include <Eigen/Dense>

#define FACTOR_MICRO 0.000001   /** @def Factor used to scale the identity matrix in update(). */
typedef Eigen::VectorXd Vector; /** @typedef typedef Eigen::VectorXd Vector */
typedef Eigen::MatrixXd Matrix; /** @typedef typedef Eigen::VectorXd Vector */

class KF
{
private:
    Vector x_prio; /// prior estimate of the state vetor
    Vector x_post; /// posterior estimate of state vector
    Matrix P_prio; /// prior estimate of the error covariance
    Matrix P_post; /// posterior estimate of the error covariance

    Vector u; /// control input vector
    Vector z; /// measurement vector
    Matrix A; /// state transition matrix
    Matrix B; /// control matrix
    Matrix C; /// measurement matrix

    Matrix Q; /// covariance matrix of model noise
    Matrix R; /// covariance matrix of measurement noise

    Matrix S; /// intermidiate matrix for computing Kalman Gain
    Matrix K; /// Kalman Gain

public:
    /**
     * @brief overloaded constructor 
     * 
     * constructs the kalman filter by specifying all its matrices and vectors
     * @param A state transition matrix
     * @param B control input matrix
     * @param C measurement matrix
     * @param Q covariance matrix of model noise
     * @param R covariance matrix of measurement noise
     * @param P initial error covariance
     * @param x initial state estimate
     * @param z initial measurements
     * @param u initial control input
     */ 
    KF(Matrix& A, Matrix& B, Matrix& C, Matrix& Q, Matrix& R, Matrix& P, Vector& x, Vector& z, Vector& u);

    /**
     * @brief default constructor
     */ 
    KF();

    /**
     * @brief overloaded constructor
     * 
     * constructs the kalman filter by specifying the dimensions of 
     * state vector, state vector and control input vector
     * @param dim_x dimension of state vector
     * @param dim_z dimension of state vector
     * @param dim_u dimension of control input vector
     */ 
    KF(int dim_x, int dim_z, int dim_u);

    /**
     * @brief default destructor
     */ 
    ~KF();

    /**
     * @brief set the state space model of the kalman filter
     * @param A desired state transition matrix
     * @param B desired control input matrix
     * @param C desired measurement matrix     
     */ 
    void set_up_model(Matrix A, Matrix B, Matrix C);

    /**
     * @brief set the covariance of measurement noise
     */ 
    void set_measure_noise(Matrix R_new);

    /**
     * @brief set the covaricance of model noise
     */ 
    void set_model_noise(Matrix Q_new);    
    
    /**
     * @brief predict step of KF given the current input vector
     * 
     * Computes the prior estimate of the state at time k 
     * based on its posterioi estimte at time k-1.
     * Computes the prior estimate of the error covariance at time k
     * based on its posterioi estimate at time k-1.
     * @param u_fresh current input vector
     */ 
    void predict(Vector u_fresh); 
    
    /**
     * @brief predict step of KF without control input
     * 
     * Computes the prior estimate of the state at time k 
     * based on its posterioi estimate at time k-1 and the linear state space model.
     * Computes the prior estimate of the error covariance at time k
     * based on its posterioi estimate at time k-1 and the linear state space model. 
     */ 
    void predict(); 
    
    /**
     * @brief update step of KF given the current measurements
     * 
     * Computes the kalman gain given new measurements.
     * Update the posterior estimate of state vector and its error covariance.
     * @param z_fresh current measurements vector
     */ 
    void update(Vector z_fresh);
    
    /**
     * @brief update step of KF without the current measurements
     * For the time where there is no measurements available yet, one must call
     * this function to keep the posterioi estimate synced to the prior estimate
     */     
    void update();
    
    /**
     * @brief show the details of KF on the console
     */ 
    void info();

    /**
     * @brief get the attribute x_prio
     */ 

    Vector get_prio_state_estm();

    /**
     * @brief get the attribute x_prio
     */
    Vector get_post_state_estm();

    /**
     * @brief get the attribute P_prio
     */    
    Matrix get_error_covariance();
};

#endif
