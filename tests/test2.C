#include "camera.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	TCamera camera;
	camera.scan();
	camera.process();

	printf("size=%d min=%f max=%f\n", camera.buffer_ptr, camera.min, camera.max);
	FILE *f = fopen("scan.jpg", "wb");
	fwrite(camera.buffer, camera.buffer_ptr, 1, f);
	fclose(f);
}
