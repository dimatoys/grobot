#include <stdio.h>

void ReverseMatrix(int n, double* matrix, double* inv){

	for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j)
                inv[i * n + j] = 1.0;
            else
                inv[i * n + j] = 0.0;
        }
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i != j) {
                double ratio = matrix[j * n + i] / matrix[i * n + i];
                for (int k = 0; k < n; k++) {
                    matrix[j * n + k] -= ratio * matrix[i * n + k];
                }
                for (int k = 0; k < n; k++) {
                    inv[j * n + k] -= ratio * inv[i * n + k];
                }
            }
        }
    }
    for (int i = 0; i < n; i++) {
        double a = matrix[i * n + i];
        for (int j = 0; j < n; j++) {
            matrix[i * n + j] /= a;
        }
        for (int j = 0; j < n; j++) {
            inv[i * n + j] /= a;
        }
    }
}

double f(double x, double y, int i) {
	//printf("%d,%d %d\n", x, y, i);
	switch(i) {
	case 0:
		return 1;
	case 1:
		return x;
	case 2:
		return y;
	case 3:
		return x * x;
	case 4:
		return y * y;
	case 5:
		return x * y;
	case 6:
		return x * x * x;
	case 7:
		return y * y * y;
	case 8:
		return x * x * y;
	case 9:
		return x * y * y;
	case 10:
		return x * x * x * x;
	case 11:
		return y * y * y * y;
	case 12:
		return x * x * y * y;
	case 13:
		return x * x * x * y;
	case 14:
		return x * y * y * y;
	}
	return 0;
}

double b(int k, int i, int width, int height) {
	double b = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			b += f(x, y, i) * f(x, y, k);
			//printf("[%d,%d]=%lf\n", x, y, b);
		}
	}
	return b;
}

void MakeImageRegressionMatrix(double* regressionMatrix, int width, int height, int n) {

	double bm[n * n];
	for (int k = 0; k < n; k++) {
		for (int i = 0; i < n; i++) {
			bm[k * n + i] = b(k, i, width, height);
		}
	}

	double inv[n * n];
	ReverseMatrix(n, bm, inv);

	double* rm = regressionMatrix;
	for (int k = 0; k < n; k++) {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				double v = 0;
				for (int i = 0; i < n; i++) {
					v += f(x, y, i) * inv[k * n + i];
				}
				*rm++ = v;
			}
		}
	}
}

void GeneratePR(int n,        // num of samples
                double* x1,   // x1[n]
                double* x2,   // x2[n]
                int k,        // regression level
                double* y,    // y[n]
                double* pr) { // PR[k]

	double* m1 = new double[k * k];
	double* pm1 = m1;

	for (int i = 0; i < k; ++i) {
		for (int j = 0; j < k; ++j) {
			double v = 0;
			for (int a = 0; a < n; ++a) {
				v += f(x1[a], x2[a], i) * f(x1[a], x2[a], j);
			}
			*pm1++ = v;
		}
	}

	double* m2 = new double[k * k];
	ReverseMatrix(k, m1, m2);
	delete m1;

	for (int i = 0; i < k; ++i) {
		double v = 0;
		for (int j = 0; j < n; ++j) {
			double w = 0;
			for (int a = 0; a < k; ++a) {
				w += m2[a + k * i] * f(x1[j], x2[j], a);
			}
			v += w * y[j];
		}
		*pr++ = v;
	}
	delete m2;
}

double Predict(double x1, double x2, int k, double* pr) {
	double v = 0;
	for (int  i = 0; i < k; ++i) {
		v += *pr++ * f(x1, x2, i);
	}
	return v;
}
