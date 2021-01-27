
void MakeImageRegressionMatrix(double* regressionMatrix, int width, int height, int n);

void GeneratePR(int n,        // num of samples
                double* x1,   // x1[n]
                double* x2,   // x2[n]
                int k,        // regression level
                double* y,    // y[n]
                double* pr);  // PR[k]

double Predict(double x1, double x2, int k, double* pr);


void GeneratePR(int     n,    // num of samples
                double* x,    // x1[0],x2[0],...,x_xn[0], x1[1],x2[1],...,x_xn[1], ... x_xn[n-1]
                int     xn,   // number of x
                int     k,    // regression level
                double* y,    // y1[0],y2[0],...,y_yn[0], y1[1],y2[1],...,y_yn[1], ... y_yn[n-1]
                int     yn,   // number of y
                double* pr);  // PR[k*yn]  k[1] 1..5 k[2] 1,3,6,10,15

void Predict(double* x, int xn, double* pr, int k, double* y, int yn);
