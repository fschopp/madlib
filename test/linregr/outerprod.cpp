    #include <stdio.h>
    #include <stdlib.h>

#ifdef USE_EIGEN
    #include <Eigen/Dense>

    using namespace Eigen;
#elif USE_ARMADILLO
    #include <armadillo>

    using namespace arma;
#endif

// We use a global variable because a small constant could give the nested
// loop an unfair advantage for compiler optimization
int len = 100;
int numiter = 1000000;

#ifdef USE_EIGEN
    template <typename Derived1, typename Derived2>
#endif
    __attribute__((noinline))
    void
    outerProduct(
#ifdef USE_EIGEN
        MatrixBase<Derived1>& mat,
        const MatrixBase<Derived2>& vec
#elif USE_ARMADILLO
        Mat<double> &mat,
        Col<double> &vec
#else
        double *mat_raw,
        double *vec_raw
#endif
    ) {
#if USE_EIGEN
        mat.noalias() += vec * vec.transpose();
#elif USE_ARMADILLO
        mat += vec * trans(vec);
#else
        for (int i = 0; i < len; i++)
            for (int j = 0; j < len; j++)
                mat_raw[i * len + j] += vec_raw[i] * vec_raw[j];
#endif
    }

    int main(int argc, char** argv) {
        if (argc < 3) {
            printf("must specify vector length and number of iterations.\n");
            exit(1);
        }
            
        len = atoi(argv[1]);
        numiter = atoi(argv[2]);
    
        double *vec_raw = (double*) malloc(len * sizeof(double));
        double *mat_raw = (double*) malloc(len * len * sizeof(double));

#ifdef USE_EIGEN
        Map<VectorXd, Unaligned> vec(vec_raw, len);
        Map<MatrixXd, Unaligned> mat(mat_raw, len, len);
#elif USE_ARMADILLO
        Col<double> vec(vec_raw, len,
            false /* copy_aux_mem */, true /* strict */);
        Mat<double> mat(mat_raw, len, len,
            false /* copy_aux_mem */, true /* strict */);
#endif
        
        // Some senseless initialization
        for (int i = 0; i < len; i++)
            vec_raw[i] = len * len - i;
        
        for (int i = 0; i < numiter; i++)
            outerProduct(
#if !defined(USE_EIGEN) && !defined(USE_ARMADILLO)
                mat_raw, vec_raw
#else
                mat, vec
#endif
            );
        
        for (int i = 0; i < len; i++)
            printf("%f ", mat_raw[i]);

        return 0;
    }
