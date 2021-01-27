#include "realsense.h"
#include "regression.h"

#include <stdio.h>

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

void videoTest() {
	TRealSense src;
	//TDepthFile src("1609729260.csv");

	TCamera camera(&src);
	camera.scan();
	//camera.showDepth();
	//camera.calibrate();
	uint8_t* data = new uint8_t[src.width * src.height * 3];
	camera.drawDepth(data);

	camera.process(data);

	camera.saveJpg(data);
	delete data;

	camera.time_stat();
	printf("max=%d size=%d buffer=%d\n", camera.max, camera.buffer_ptr, camera.buffer_size);

	FILE *f = fopen("scan.jpg", "wb");
	fwrite(camera.buffer, camera.buffer_ptr, 1, f);
	fclose(f);
}


int main(int argc, char **argv)
{

	PRtest();
	PRtest1();
	PRtest2();
	//videoTest();

}
