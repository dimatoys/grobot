#include "camera.h"
#include "regression.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <time.h>
#include <stdlib.h>

void TDepth::save() {

	char file_name[100];
	sprintf(file_name,"depth%lu.csv", time(NULL));
	FILE* f = fopen(file_name, "w");
	for (auto y = 0; y < height; ++y) {
		char* c = "";
		for (auto x = 0; x < width; ++x) {
			float d = get_distance(x, y);
			fprintf(f, "%s%f",c,d);
			c = ", ";
		}
		fputc('\n', f);
	}
	fclose(f);
}

TDepthFile::~TDepthFile() {
	if (data != NULL) {
		delete data;
	}
}

void TDepthFile::newFrame() {
	if (data != NULL) {
		delete data;
		data = NULL;
	}
	
	char line[20000];
	
	FILE* f = fopen(fileName.c_str(), "r");
	height = 0;
	int buffer_ptr = 0;
	int buffer_size = 640 * 480;
	data = new float[buffer_size];
	while(fgets(line,sizeof line,f)!= NULL) {
		char* begin = line;
		char* end = line;
		while (*end != '\n') {
			if (buffer_ptr == buffer_size) {
				buffer_size += 10000;
				float* new_buffer = new float[buffer_size];
				memcpy(new_buffer, data, buffer_ptr * sizeof(float));
				delete data;
				data = new_buffer;
			}
			data[buffer_ptr++] = strtof(begin, &end);
			begin = end + 1;
		}
		height++;
		//printf("%d: %d\n", height, buffer_ptr);
	}
	width = buffer_ptr / height;
	
	printf("%d x %d\n", width, height);
	
	fclose(f);
}

TCamera::TCamera(TDepth* depth) {

	buffer_size = 640*480;
	buffer = new uint8_t[buffer_size];

	limit = 0.6;

	this->depth = depth;
}

void TCamera::resetBuffer() {
	buffer_ptr = 0;
}

void TCamera::writeBuffer(const void* data, int size) {
	if (buffer_ptr + size > buffer_size) {
		buffer_size += size + 10000;
		auto new_buffer = new uint8_t[buffer_size];
		memcpy(new_buffer, buffer, buffer_ptr);
		delete buffer;
		buffer = new_buffer;
	}
	memcpy(buffer + buffer_ptr, data, size);
	buffer_ptr += size;
}

void TCamera::scan() {
	
	depth->newFrame();

	//depth->save();
}

#define AREA(x,y) (x >= 92) && (x + y >= 302) && (y-x >= 130-848) && ((y <= 340) || (x<= 750 )) && ((y <= 422) || (x <= 350) || (x >= 640))

void TCamera::process() {

	int k = 10;
	
	int step = 10;
	double x1[depth->width * depth->height / step / step];
	double x2[depth->width * depth->height / step / step];
	double y1[depth->width * depth->height / step / step];

	int n = 0;
	for (int y = 0; y < depth->height; y += step) {
		for (int x = 0; x < depth->width; x += step) {
			if (AREA(x,y)) {
				auto d = depth->get_distance(x, y);
				if (d > 0) {
					x1[n] = x - depth->width / 2;
					x2[n] = y - depth->height / 2;
					y1[n++] = d;
				}
			}
		}
	}

	double pr[k];
	GeneratePR(n, x1, x2, k, y1, pr);

	for (int i = 0; i < k; ++i) {
		printf("%d: %20.15f\n", i, pr[i]);
	}


	max = 0;
	min = limit;

	for (int y = 0; y < depth->height; ++y) {
		for (int x = 0; x < depth->width; ++x) {
			float d = depth->get_distance(x, y);
			if (d > 0) {
				if (d < min) {
					min = d;
				}
				if (d > max) {
					max = d;
				}
			}
		}
	}

	if (max > limit) {
		max = limit;
	}

	auto interval = max - min;

	resetBuffer();
	if (interval > 0) {

		int channels = 3;
		uint8_t* data = new uint8_t[depth->width * depth->height * channels];
		auto p = data;
		for (auto y = 0; y < depth->height; ++y) {
			for (auto x = 0; x < depth->width; ++x) {
				float d = depth->get_distance(x, y);
				if (d > limit) {
					d = limit;
				}
					/*
					uint8_t color = d > 0 ? (uint8_t)(255 * (d - min) / interval) : 0;
					*p++ = color;
					*p++ = color;
					*p++ = color;
					*/
					if (d > 0) {
						if (fabs(d - Predict(x - depth->width / 2, y - depth->height / 2, k, pr)) < 0.006) {
							*p++ = 100;
							*p++ = 255;
							*p++ = 100;
						} else {
							*p++ = 255;
							*p++ = 100;
							*p++ = 100;
						}
					} else {
						*p++ = 0;
						*p++ = 0;
						*p++ = 0;
					}
			}
		}

		stbi_write_jpg_to_func(
			[](void *context, void *compressed, int size) {
				((TCamera*)context)->writeBuffer(compressed, size);
			}, this, depth->width, depth->height, channels, data, 90);
		delete data;
	}
}
