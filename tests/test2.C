#include "regression.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <cstdint>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

void PRtest() {

	int n = 3;
	double x1[] = {0, 0, 1};
	double x2[] = {0, 1, 0};
	int k = 3;
	double y[]  = {0, 0, 1};
	
	double pr[k];

	GeneratePR(n, x1, x2, k, y, pr);

	for (int i = 0; i < k; ++i) {
		printf("%f\n", pr[i]);
	}

	double tx1 = 3.0;
	double tx2 = 4.0;
	printf("(%f,%f) -> %f\n", tx1, tx2, Predict(tx1,tx2,k, pr));

}

void PRtest1() {
	int n = 3;           	// num of samples
	double x[] = {0,0, 0,1, 1,0};	// x1[0],x2[0],...,x_xn[0], x1[1],x2[1],...,x_xn[1], ... x_xn[n-1]
	int xn = 2;          	// number of x
	int k = 3;           	// regression level
	double y[] = {0,0,1};	// y1[0],y2[0],...,y_yn[0], y1[1],y2[1],...,y_yn[1], ... y_yn[n-1]
	int yn = 1;         	// number of y

	double pr[k*yn];       	// PR[k*yn]  k[1] 1..5 k[2] 1,3,6,10,15

	GeneratePR(n, x, xn, k, y, yn, pr);

	for (int i = 0; i < k * yn; ++i) {
		printf("%f\n", pr[i]);
	}

	double tx[] = {3.0, 4.0};
	double ty;
	Predict(tx, xn, pr, k, &ty, yn);

	printf("(%f,%f) -> %f\n", tx[0], tx[1], ty);
}

void PRtest2() {
	int n = 5;           	// num of samples
	double x[] = {1,2,3,4,5};	// x1[0],x2[0],...,x_xn[0], x1[1],x2[1],...,x_xn[1], ... x_xn[n-1]
	int xn = 1;          	// number of x
	int k = 5;           	// regression level
	double y[] = {3,5,7,9,11};	// y1[0],y2[0],...,y_yn[0], y1[1],y2[1],...,y_yn[1], ... y_yn[n-1]
	int yn = 1;         	// number of y

	double pr[k*yn];       	// PR[k*yn]  k[1] 1..5 k[2] 1,3,6,10,15

	GeneratePR(n, x, xn, k, y, yn, pr);

	for (int i = 0; i < k * yn; ++i) {
		printf("%f\n", pr[i]);
	}

	double tx = 10;
	double ty;
	Predict(&tx, xn, pr, k, &ty, yn);

	printf("(%f) -> %f\n", tx, ty);

}

void testF(double* x, double* y) {
	y[0] = x[0] + 2 * x[1];
	y[1] = x[0] + x[1];
}


void rtest1(){
	double x0[2] = {10,5};
	double y[2];
	testF(x0, y);

	double x[2] = {0,0};
	Reverse(testF,y, 2, x, 2, 0.1);
	
	printf("%f,%f\n", x[0], x[1]);
}

double testBF(double* x, double* p) {
	return x[0] * p[0] + x[1] * p[1];
}

void bftest1() {

	double x[] = {0,0, 0,1, 1,1};
	double p0[] = {3,4};
	double y[] = {testBF(x + 0, p0), testBF(x + 2, p0), testBF(x + 4, p0)};
	double p[] = {0,0};

	double d =  BestFit(testBF, 3, 2, x, y, 2, p, 0.1);

	printf("%f %f d=%f\n", p[0], p[1], d);
}

/*
void ABtoPOS(double* x, double* y) {
    double R1 = 175;
    double R2 = 40;
    double R3 = 265;
    double R4 = 59;
    float a = M_PI * (x[0] - 500) / 1000;
    float b = M_PI * (x[1] - 500) / 1000;

	float PI = M_PI;

    y[0] = R1 * sin(b) + R2 * sin(b - PI / 4) + R3 * sin(a + b - PI / 4) + R4 * sin(a + b - PI / 4 - PI / 2);
    y[1] = R1 * cos(b) + R2 * cos(b - PI / 4) + R3 * cos(a + b - PI / 4) + R4 * cos(a + b - PI / 4 - PI / 2);
}
*/

void ABtoPOS(double* x, double* y) {
    double R1 = 175;
    double R2 = 40;
    double R3 = 265;
    double R4 = 59;
    double a = M_PI * (x[0] - 500) / 1000;
    double b = M_PI * (x[1] - 500) / 1000;

    y[0] = R1 * sin(b) + R2 * sin(b - M_PI / 4) + R3 * sin(a + b - M_PI / 4) + R4 * sin(a + b - M_PI / 4 - M_PI / 2);
    y[1] = R1 * cos(b) + R2 * cos(b - M_PI / 4) + R3 * cos(a + b - M_PI / 4) + R4 * cos(a + b - M_PI / 4 - M_PI / 2);
}

struct TGRobot {
	double* surfacePosPr;
	int		surfacePosK;
};

void generateABPR(TGRobot* grobot, int k) {
	double ab[] = {	1000, 200,
					 800, 300,
					 900, 300,
					1000, 300,
					 300, 500,
					 500, 500,
					 700, 500,
					 900, 500,
					1000, 500,
					 100, 600,
					 300, 600,
					 500, 600,
					 700, 600,
					 900, 600,
					1000, 600,
					   0, 800,
					 100, 800,
					 300, 800,
					 500, 800,
					   0,1000,
					 100,1000};
	double pos[] = {
					-308,283,
					-331,324,
					-248,369,
					-166,383,
					-300,170,
					-253,354,
					 -97,459,
					  84,437,
					 151,389,
					-196,79,
					-213,274,
					 -96,425,
					  86,461,
					 245,371,
					 290,300,
					 -96,96,
					 -78,199,
					  47,347,
					 243,364,
					   5,135,
					  85,195
};
	int n = sizeof(ab) / sizeof(ab[0]) / 2;

	double* x = new double[n*2];
	for (int i = 0; i < n;++i) {
		x[i*2] = ab[i*2];
		x[i*2+1] = ab[i*2+1];
		Reverse(ABtoPOS,pos +i*2, 2, x + i*2, 2, 0.1);
	}
	// regression level, k[yn=1] 1..5 k[yn=2] 1,3,6,10,15
	grobot->surfacePosK = k;
	grobot->surfacePosPr = new double[k*2];

	GeneratePR(n, ab, 2, grobot->surfacePosK, x, 2, grobot->surfacePosPr);
/*
	for (int i = 0; i < n; ++i) {
		double xc[2];
		double y[2];
		Predict(ab + i*2, 2, grobot->surfacePosPr, grobot->surfacePosK, xc, 2);
		ABtoPOS(xc,y);
		printf("%f %f -> %f %f -> %f %f (%f %f -> %f %f) %f\n", ab[i*2], ab[i*2+1], xc[0], xc[1], y[0], y[1], x[i*2], x[i*2+1], pos[i*2], pos[i*2+1],
			sqrt((y[0] -pos[i*2]) * (y[0] -pos[i*2]) + (y[1] -pos[i*2+1]) * (y[1] -pos[i*2+1])));
	}
*/
	delete x;

	double abc[2];
/*
	double xmin = 0;
	double xmax = 0;
	double ymin = 200;
	double ymax = 200;
*/

	int center = 477;
	int width = center*2;
	int height = 476;
	uint8_t map[width * height];
	memset(map, 0, width * height);

	
	for (abc[0] = 0.0; abc[0] <= 1000.0; abc[0] += 1.0) {
		for (abc[1] = 0.0; abc[1] <=1000.0; abc[1] += 1.0) {
			double abm[2];
			double xy[2];
			Predict(abc, 2, grobot->surfacePosPr, grobot->surfacePosK, abm, 2);
			ABtoPOS(abm,xy);

			int px = xy[0] + center;
			int py = xy[1];

			if (py >= 0) {
				map[px+py*width] = 255;

/*
				if (xy[0] < xmin) {
					xmin = xy[0];
				} else {
					if (xy[0] > xmax) {
						xmax = xy[0];
				}	
				}
				if (xy[1] < ymin) {
					ymin = xy[1];
				} else {
					if (xy[1] > ymax) {
						ymax = xy[1];
					}
				}
*/
			}
		}
	}
	//printf("x=[%f..%f] y=[%f..%f]\n", xmin, xmax, ymin, ymax);
	// x=[-476.730587..476.730681] y=[-456.568562..476.730685]


	FILE *f = fopen("cover.jpg", "wb");

	stbi_write_jpg_to_func(
	[](void *context, void *compressed, int size) {
		fwrite(compressed, size, 1, (FILE*)context);
	}, f, width, height, 1, map, 90);

	fclose(f);

}


void rtest2(){
	double ab[] = {	1000, 200,
					 800, 300,
					 900, 300,
					1000, 300,
					 300, 500,
					 500, 500,
					 700, 500,
					 900, 500,
					1000, 500,
					 100, 600,
					 300, 600,
					 500, 600,
					 700, 600,
					 900, 600,
					1000, 600,
					   0, 800,
					 100, 800,
					 300, 800,
					 500, 800,
					   0,1000,
					 100,1000};
	double pos[] = {
					-308,283,
					-331,324,
					-248,369,
					-166,383,
					-300,170,
					-253,354,
					 -97,459,
					  84,437,
					 151,389,
					-196,79,
					-213,274,
					 -96,425,
					  86,461,
					 245,371,
					 290,300,
					 -96,96,
					 -78,199,
					  47,347,
					 243,364,
					   5,135,
					  85,195
};
	int n = sizeof(ab) / sizeof(ab[0]) / 2;

	printf("array=%ld d=%ld %d\n", sizeof(ab),sizeof(ab[0]), n);

	double* x = new double[n*2];
	for (int i = 0; i < n;++i) {
		x[i*2] = ab[i*2];
		x[i*2+1] = ab[i*2+1];
		Reverse(ABtoPOS,pos +i*2, 2, x + i*2, 2, 0.1);
	}
	// regression level, k[yn=1] 1..5 k[yn=2] 1,3,6,10,15
	int k = 6;
	double* pr = new double[k*2];

	GeneratePR(n, ab, 2, k, x, 2,pr);

	for (int i = 0; i < n; ++i) {
		double xc[2];
		double y[2];
		Predict(ab + i*2, 2, pr, k, xc, 2);
		ABtoPOS(xc,y);
		printf("%f %f -> %f %f -> %f %f (%f %f -> %f %f) %f\n", ab[i*2], ab[i*2+1], xc[0], xc[1], y[0], y[1], x[i*2], x[i*2+1], pos[i*2], pos[i*2+1],
			(double)sqrt((long double)((y[0] -pos[i*2]) * (y[0] -pos[i*2]) + (y[1] -pos[i*2+1]) * (y[1] -pos[i*2+1]))));
	}

}

void rtest3(){
	TGRobot grobot;
	generateABPR(&grobot, 6);
}

int main(int argc, char **argv)
{

	//PRtest();
	//PRtest1();
	//PRtest2();
	//videoTest();
	//rtest1();
	bftest1();
	//rtest2();
	//rtest3();
}
