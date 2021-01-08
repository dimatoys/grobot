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

	reg_k = 10;
	reg_pr = new double[reg_k];
	reg_pr[0] = 0.358500017400887;
	reg_pr[1] = 0.000017839485073;
	reg_pr[2] = -0.000534426058830;
	reg_pr[3] = -0.000000006821775;
	reg_pr[4] =  0.000000850294317;
	reg_pr[5] = -0.000000065215744;
	reg_pr[6] =  0.000000000007386;
	reg_pr[7] = -0.000000001221031;
	reg_pr[8] =  0.000000000035413;
	reg_pr[9] =  0.000000000108109;
	reg_surface = NULL;
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

#define AREA(x, y) (x >= 92) && (x + y >= 302) && (y-x >= 130-848) && ((y <= 340) || (x<= 750 )) && ((y <= 422) || (x <= 350) || (x >= 640))

void TCamera::showDepth() {
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
		uint8_t* data = new uint8_t[depth->width * depth->height];
		auto p = data;
		for (auto y = 0; y < depth->height; ++y) {
			for (auto x = 0; x < depth->width; ++x) {
				float d = depth->get_distance(x, y);
				if (d > limit) {
					d = limit;
				}
				*p++ = d > 0 ? (uint8_t)(255 * (d - min) / interval) : 0;
			}
		}
		stbi_write_jpg_to_func(
			[](void *context, void *compressed, int size) {
				((TCamera*)context)->writeBuffer(compressed, size);
			}, this, depth->width, depth->height, 1, data, 90);
		delete data;
	}
}

void TCamera::calibrate() {

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

	GeneratePR(n, x1, x2, reg_k, y1, reg_pr);

	for (int i = 0; i < reg_k; ++i) {
		printf("%d: %20.15f\n", i, reg_pr[i]);
	}

	resetBuffer();

	int channels = 3;
	uint8_t* data = new uint8_t[depth->width * depth->height * channels];
	auto p = data;
	for (int y = 0; y < depth->height; ++y) {
		for (int x = 0; x < depth->width; ++x) {
			float d = depth->get_distance(x, y);
			if (d > 0) {
				if (fabs(d - Predict(x - depth->width / 2, y - depth->height / 2, reg_k, reg_pr)) < 0.006) {
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

void TCamera::process() {
	
	int height = 434;
	
	if (reg_surface == NULL) {
		reg_surface = new double[depth->width * depth->height];
		auto r = reg_surface;
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < depth->width; ++x) {
				*r++ = Predict(x - depth->width / 2, y - depth->height / 2, reg_k, reg_pr);
			}
		}
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
	uint8_t* data = new uint8_t[depth->width * depth->height * 3];
	auto p = data;
	auto r = reg_surface;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < depth->width; ++x) {
			float d = depth->get_distance(x, y);
			if (d > 0) {
				uint8_t b = (uint8_t)(d < max ? (255 * (d  - min) / interval) : 255);

				auto h = *r - d;
				if (h > 0) {
					if (h >= 0.01) {
						*p++ = 255;
						*p++ = 100;
						*p++ = 100;
					} else {
						*p++ = b;
						*p++ = b;
						*p++ = b;
					}
				} else {
					*p++ = b;
					*p++ = b;
					*p++ = b;
				}
			} else {
				*p++ = 0;
				*p++ = 0;
				*p++ = 0;
			}
			r++;
		}
	}
	for (int y = height; y < depth->height; ++y) {
		for (int x = 0; x < depth->width; ++x) {
			float d = depth->get_distance(x, y);
			uint8_t b = (uint8_t)(255 * (d - min) / interval);
			*p++ = b;
			*p++ = b;
			*p++ = b;
		}
	}
	
	
	stbi_write_jpg_to_func(
		[](void *context, void *compressed, int size) {
			((TCamera*)context)->writeBuffer(compressed, size);
		}, this, depth->width, depth->height, 3, data, 90);
	delete data;
}
