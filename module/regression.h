
void MakeImageRegressionMatrix(double* regressionMatrix, int width, int height, int n);

void GeneratePR(int n,        // num of samples
                double* x1,   // x1[n]
                double* x2,   // x2[n]
                int k,        // regression level
                double* y,    // y[n]
                double* pr);  // PR[k]

double Predict(double x1, double x2, int k, double* pr);

