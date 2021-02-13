#include "realsense.h"
#include "regression.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

long get_diff(struct timespec& start, struct timespec& end) {
	return (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
}

void perfTest() {

	struct timespec start_scan;
	struct timespec image_ready;
	struct timespec drawn;
	struct timespec init;
	struct timespec processed;
	struct timespec saved;

	TRealSense src;
	//TDepthFile src("1609729260.csv");

	TCamera camera;

	clock_gettime(CLOCK_REALTIME, &start_scan);

	src.newFrame();
	clock_gettime(CLOCK_REALTIME, &image_ready);

	//camera.calibrate(&src);
	camera.drawDepth(&src);
	clock_gettime(CLOCK_REALTIME, &drawn);

	camera.processInit(&src);
	clock_gettime(CLOCK_REALTIME, &init);

	camera.process(&src);
	clock_gettime(CLOCK_REALTIME, &processed);

	camera.saveJpg();
	FILE *f = fopen("scan.jpg", "wb");
	fwrite(camera.buffer, camera.buffer_ptr, 1, f);
	fclose(f);
	clock_gettime(CLOCK_REALTIME, &saved);


	printf("scan:       %-12.9f\n", (double)get_diff(start_scan, image_ready)/ 1000000000.0);
	printf("draw:       %-12.9f\n", (double)get_diff(image_ready, drawn)/ 1000000000.0);
	printf("init:       %-12.9f\n", (double)get_diff(drawn, init)/ 1000000000.0);
	printf("process:    %-12.9f\n", (double)get_diff(init, processed)/ 1000000000.0);
	printf("save:       %-12.9f\n", (double)get_diff(processed, saved)/ 1000000000.0);
	printf("\ntotal:      %-12.9f\n", (double)get_diff(start_scan, saved)/ 1000000000.0);
}

void videoTest() {
	TRealSense src;
	//TDepthFile src("1609729260.csv");

	TCamera camera;

	src.newFrame();

	camera.drawDepth(&src);
	//camera.calibrate(&src);
	camera.calibrate2(&src);

	//camera.processInit(&src);

	//camera.process(&src);

	camera.saveJpg();
	FILE *f = fopen("scan.jpg", "wb");
	fwrite(camera.buffer, camera.buffer_ptr, 1, f);
	fclose(f);
	
	char c = '[';
	for (int y= 0; y < src.height; ++y) {
		printf("%c%d", c, src.get_distance_mm(src.width / 2, y));
		c = ',';
	}
	printf("]\n");
}

int main(int argc, char **argv)
{

	//perfTest();
	videoTest();
}
