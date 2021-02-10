
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
                int     xn,   // dimensions of x
                int     k,    // regression level, k[yn=1] 1..5 k[yn=2] 1,3,6,10,15
                double* y,    // y1[0],y2[0],...,y_yn[0], y1[1],y2[1],...,y_yn[1], ... y_yn[n-1]
                int     yn,   // dimensions of y
                double* pr);  // output:PR[k*yn]

void Predict(double* x,       // x - value
                int xn,       // dimensions of x
             double* pr,      // PR[k*yn]
                 int k,       // regression level, k[yn=1] 1..5 k[yn=2] 1,3,6,10,15
             double* y,       // output: y
              int yn);        // dimensions of y

double Reverse(void (*f)(double*,double*), //function to reverse (x,y)
               double* y,                  // y - value
               int yn,                     // dimensions of y
               double* x,                  // output: x - value, input: initial value
               
               int xn,                     // dimensions of x
               double step);               // gradient step
                                           // returns the distance

double BestFit(double (*f)(double*,double*), // function (x,p) -> y
               int n,                        // number of samples
               int xn,                       // dimensions of x
               double* x,                    // x1[0],x2[0],...,x_xn[0], x1[1],x2[1],...,x_xn[1], ... x_xn[n-1]
               double* y,                    // y1, ... yn
               int pn,                       // number of parameters
               double* p,                    // input: init parameters, output: best fit parameters
               double step);                 // gradient step
                                             // returns: the distance
