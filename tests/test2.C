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
}

int main(int argc, char **argv)
{

	//PRtest();

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
