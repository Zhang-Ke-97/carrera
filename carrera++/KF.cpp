#include "KF.h"
#include <iostream>

KF::KF(Matrix &A, Matrix &B, Matrix &C, Matrix &Q, Matrix &R, Matrix &P, Vector &x, Vector &z, Vector &u):
A(A), B(B), C(C), Q(Q), R(R), P_post(P), x_post(x), z(z), u(u){
    int dim_x = x.rows();
    int dim_z = z.rows();
    P_prio = Matrix::Zero(dim_x, dim_x);
    x_prio = Vector::Zero(dim_x, 1);
    S = Matrix::Zero(dim_z, dim_z);
    K = Matrix::Zero(dim_x, dim_z);
}

KF::KF(int dim_x, int dim_z, int dim_u){
    A = Matrix::Zero(dim_x, dim_x);
    B = Matrix::Zero(dim_x, dim_u);
    C = Matrix::Zero(dim_z, dim_x);

    Q = Matrix::Identity(dim_x, dim_x);
    R = Matrix::Identity(dim_z, dim_z);

    P_prio = 1000*Matrix::Identity(dim_x, dim_x);
    P_post = 1000*Matrix::Identity(dim_x, dim_x);
    
    x_prio = Vector::Zero(dim_x, 1);
    x_post = Vector::Zero(dim_x, 1);

    z = Vector::Zero(dim_z, 1);
    u = Vector::Zero(dim_u, 1);

    S = Matrix::Zero(dim_z, dim_z);
    K = Matrix::Zero(dim_x, dim_z);
}

void KF::predict(Vector u_fresh){
    this->u = u_fresh;
    x_prio = A * x_post + B * u;
    P_prio = A * P_post * A.transpose() + Q;
}

void KF::predict(){
    x_prio = A * x_post + B * u;
    P_prio = A * P_post * A.transpose() + Q;
}

void KF::update(Vector z_fresh){
    this->z = z_fresh;
    // make sure that S is always invertiable
    S = C * P_prio * C.transpose() + R + FACTOR_MICRO*Matrix::Identity(z.rows(), z.cols());
    
    K = P_prio * C.transpose() * S.inverse();
    x_post = x_prio + K * (z - C * x_prio);
    P_post = P_prio - K * S * K.transpose();
}

Vector KF::get_prio_state_estm(){
    return this->x_prio;
}

Vector KF::get_post_state_estm(){
    return this->x_post;
}

Matrix KF::get_error_covariance(){
    return this->P_post;
}

void KF::info(){
    Eigen::IOFormat fmt(4, 0, "\t", "\n", "\t[", "]"); // set matrix print-layout
    std::string line_sep = "\n-------------------------------------\n";
    std::cout << "KF filter created:" << std::endl;
    std::cout << "\tx = Ax + Bu + w" << std::endl;
    std::cout << "\tz = Cx + v" << line_sep;
    std::cout << "A=\n" << this->A.format(fmt) << line_sep;
    std::cout << "B=\n" << this->B.format(fmt) << line_sep;
    std::cout << "C=\n" << this->C.format(fmt) << line_sep;
    std::cout << "covariance of model noise w:\n" << this->Q.format(fmt) << line_sep;
    std::cout << "covariance of measurement noise v:\n" << this->R.format(fmt) << line_sep;
}

void KF::set_measure_noise(Matrix R_new){
    if(this->R.rows()==R_new.rows() && this->R.cols()==R_new.cols()){
        this->R << R_new;
    }else{
        std::cout << "Warning: The size of meausurement noise cov doesn't match!\n";
    }
}

void KF::set_model_noise(Matrix Q_new){
    if(this->Q.rows()==Q_new.rows() && this->Q.cols()==Q_new.cols()){
        this->Q << Q_new;
    }else{
        std::cout << "Warning: The size of model noise cov doesn't match!\n";
    }
}


void KF::set_up_model(Matrix A, Matrix B, Matrix C){
    this->A << A;
    this->B << B;
    this->C << C;
}

KF::KF(){
}

KF::~KF(){
}
