#include <stdio.h>
#include <math.h>

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

typedef double (*FT)(double*);

double f0(double* args) {
	return 1;
}

double f1_1(double* args) {
	return *args;
}

double f1_2(double* args) {
	auto x = *args;
	return x * x;
}

double f1_3(double* args) {
	auto x = *args;
	return x * x * x;
}

double f1_4(double* args) {
	auto x = *args;
	auto x2 = x * x;
	return x2 * x2;
}

double f2_1(double* args) {
	auto y = *(args + 1);
	return y;
}

double f2_3(double* args) {
	auto y = *(args + 1);
	return y * y;
}

double f2_4(double* args) {
	return *args * *(args + 1);
}

double f2_6(double* args) {
	auto y = *(args + 1);
	return y * y * y;
}

double f2_7(double* args) {
	auto x = *args;
	auto y = *(args + 1);
	return y * y * x;
}

double f2_8(double* args) {
	auto x = *args;
	auto y = *(args + 1);
	return y * x * x;
}

double f2_10(double* args) {
	auto y = *(args + 1);
	auto y2 = y * y;
	return y2 * y2;
}

double f2_11(double* args) {
	auto x = *args;
	auto y = *(args + 1);
	return y * y * y * x;
}

double f2_12(double* args) {
	auto x = *args;
	auto y = *(args + 1);
	return y * y * x * x;
}

double f2_13(double* args) {
	auto x = *args;
	auto y = *(args + 1);
	return y * x * x * x;
}

const static FT pf[][15] = {{f0, f1_1, f1_2, f1_3, f1_4},
                            {f0, f2_1, f1_1, f2_3, f2_4, f1_2, f2_6, f2_7, f2_8, f1_3, f2_10, f2_11, f2_12, f2_13, f1_4}};

void GeneratePR(int     n,    // num of samples
                double* x,    // x1[0],x2[0],...,x_xn[0], x1[1],x2[1],...,x_xn[1], ... x_xn[n-1]
                int     xn,   // number of x
                int     k,    // regression level
                double* y,    // y1[0],y2[0],...,y_yn[0], y1[1],y2[1],...,y_yn[1], ... y_yn[n-1]
                int     yn,   // number of y
                double* pr) { // PR[k*yn]  k[1] 1..5 k[2] 1,3,6,10,15

	const FT* p = pf[xn - 1];

	double* m1 = new double[k * k];
	double* pm1 = m1;

	for (int i = 0; i < k; ++i) {
		for (int j = 0; j < k; ++j) {
			double v = 0;
			for (int a = 0; a < n; ++a) {
				v += p[i](x + a * xn) * p[j](x + a * xn);
				//printf("%f(%d,%d) * %f(%d,%d) <- %f[%d]\n", p[i](x + a * xn), i,a,p[j](x + a * xn),j,a,*(x + a * xn), a*xn);
			}
			*pm1++ = v;
			//printf("-> %f [%d,%d]\n", v, j,i);
		}
	}


	double* m2 = new double[k * k];
	ReverseMatrix(k, m1, m2);
	delete m1;

	for (int i = 0; i < k; ++i) {
		for (int j = 0; j < yn; ++j) {
			pr[j] = 0;
		}
		for (int j = 0; j < n; ++j) {
			double w = 0;
			for (int a = 0; a < k; ++a) {
				w += m2[a + k * i] * p[a](x + j * xn);
				//printf(" + %f * %f", m2[a + k * i], p[a](x + j * xn));
			}
			//printf("-> %f\n", w);
			for(int t = 0; t < yn; ++t) {
				//auto old = pr[t];
				pr[t] += w * y[j * yn + t];
				//printf("pr[%d] = %f + %f * %f -> %f\n", i*yn+t, old, w, y[j * yn + t], pr[t]);
			}
		}
		pr += yn;
	}
	delete m2;

}

void Predict(double* x, int xn, double* pr, int k, double* y, int yn) {

	const FT* p = pf[xn - 1];

	for (int i = 0; i < yn; ++i) {
		y[i] = 0;
	}

	for (int i = 0; i < k; ++i) {
		auto r = p[i](x);
		for (int j =0; j < yn; ++j) {
			//printf(" + %f * %f", *pr, r);
			y[j] += *pr++ * r;
		}
	}
	//printf("\n");
}

double Reverse(void (*f)(double*,double*), double* y, int yn, double* x, int xn, double step) {
	int dirs = (int)pow(3,xn)-1;
	double* dir = new double[dirs * xn];
	for (int i = 0; i < xn; ++i){
		dir[i] = -step;
	}
	for (int i = 1; i<dirs; ++i) {
		double add = step;
		bool allZero = true;
		for (int j = 0; j < xn; ++j) {
			double v = dir[(i-1)*xn+j];
			double nv;
			if (add > 0) {
				if(v <= 0) {
					nv = v + add;
					add = 0;
				} else {
					nv = -step;
				}
			} else {
				nv = v;
			}
			dir[i * xn + j] = nv;
			allZero &= (nv == 0.0);
		}
		if (allZero) {
			dir[i*xn] = step;
		}
	}
	double* yc = new double[yn];
	
	f(x,yc);
	
	double d0 = 0;
	for (int  i = 0; i < yn; ++i) {
		auto di = y[i] - yc[i];
		d0+= di * di;
	}
	
	double* xc = new double[xn];
	while(true){
		int i2 = -1;
		double d2 = d0;
		for (int i = 0; i < dirs; ++i) {
			for (int j = 0; j < xn; ++j) {
				xc[j] = x[j] + dir[i * xn + j];
			}
			f(xc,yc);
			double dc = 0;
			for (int j = 0; j < yn; ++j) {
				auto di = y[j] - yc[j];
				dc+= di * di;
			}
			if (dc < d2) {
				d2 = dc;
				i2 = i;
			}
		}
		if (i2 < 0) {
			break;
		}

		for (int  j =0; j < xn; ++j) {
			x[j] += dir[i2 * xn + j];
		}
		d0 = d2;
	}
	delete dir;
	delete yc;
	delete xc;
	
	return d0;
	
}

double BestFit(double (*f)(double*,double*), int n, int xn, double* x, double* y, int pn, double* p, double step) {
	int dirs = (int)pow(3,pn)-1;
	double* dir = new double[dirs * pn];
	for (int i = 0; i < pn; ++i){
		dir[i] = -step;
	}
	for (int i = 1; i<dirs; ++i) {
		double add = step;
		bool allZero = true;
		for (int j = 0; j < pn; ++j) {
			double v = dir[(i-1)*pn+j];
			double nv;
			if (add > 0) {
				if(v <= 0) {
					nv = v + add;
					add = 0;
				} else {
					nv = -step;
				}
			} else {
				nv = v;
			}
			dir[i * pn + j] = nv;
			allZero &= (nv == 0.0);
		}
		if (allZero) {
			dir[i*pn] = step;
		}
	}

	double d0 = 0;
	for (int i = 0; i < n; ++i) {
		double dc = f(x + i * xn, p) - y[i];
		d0 += dc * dc;
	}

	double* pc = new double[pn];
	while(true){
		auto d2 = d0;
		int i2 = -1;
		for (int i = 0; i < dirs; ++i) {
			for (int j = 0; j < pn; ++j) {
				pc[j] = p[j] + dir[pn *i + j];
			}
			double d1 = 0;
			for (int j = 0; j < n; ++j) {
				double dc = f(x + j * xn, pc) - y[j];
				d1+= dc * dc;
			}
			if (d1 < d2) {
				d2 = d1;
				i2 = i;
			}
		}
		if (i2 < 0) {
			break;
		}
		for (int j = 0; j < pn; ++j) {
			p[j] += dir[pn *i2 + j];
		}
		d0 = d2;
	}
	delete dir;
	delete pc;
	
	return d0;
}
