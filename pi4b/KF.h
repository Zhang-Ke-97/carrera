#ifndef KF_H
#define KF_H

#include <Eigen/Dense>

#define FACTOR_MICRO 0.000001
typedef Eigen::VectorXd Vector;
typedef Eigen::MatrixXd Matrix;

class KF
{
private:
    Vector x_prio; // prior estimate of the state vetor
    Vector x_post; // posterior estimate of state vector
    Matrix P_prio; // prior estimate of the error covariance
    Matrix P_post; // posterior estimate of the error covariance

    Vector u; // control input vector
    Vector z; // measurement vector
    Matrix A; // state transition matrix
    Matrix B; // control matrix
    Matrix C; // measurement matrix

    Matrix Q; // covariance matrix for model noise
    Matrix R; // covariance matrix for measurement noise

    Matrix S; // intermidiate matrix for computing Kalman Gain
    Matrix K; // Kalman Gain

public:
    KF(Matrix& A, Matrix& B, Matrix& C, Matrix& Q, Matrix& R, Matrix& P, Vector& x, Vector& z, Vector& u);
    KF();
    KF(int dim_x, int dim_z, int dim_u);
    ~KF();
    void set_up_model(Matrix A, Matrix B, Matrix C);
    void set_measure_noise(Matrix R_new);
    void set_model_noise(Matrix Q_new);    
    
    // predict with control input
    void predict(Vector u_fresh); 
    
    // predict without control input
    void predict(); 
    
    // update with measurements
    void update(Vector z_fresh);
    
    // update without measurements (To keep x_post updated)
    void update();
    
    void info();
    Vector get_prio_state_estm();
    Vector get_post_state_estm();
    Matrix get_error_covariance();
};

#endif
