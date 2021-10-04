#ifndef KF_H
#define KF_H

#include <Eigen/Dense>

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
    void predict(Vector u_fresh); // predict with control input
    void predict(); // predict without control input
    void update(Vector z_fresh);
    void info();
    Vector get_prio_state_estm();
    Vector get_post_state_estm();
    Matrix get_error_covariance();
    void set_measure_noise(Matrix R_new);
    void set_model_noise(Matrix Q_new);
};

#endif