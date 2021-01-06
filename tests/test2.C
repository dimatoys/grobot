#include "realsense.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	//TRealSense src;
	TDepthFile src("1609729260.csv");
	
	TCamera camera(&src);
	camera.scan();
	camera.process();

	printf("min=%f max=%f\n", camera.min, camera.max);
	FILE *f = fopen("scan.jpg", "wb");
	fwrite(camera.buffer, camera.buffer_ptr, 1, f);
	fclose(f);
}
