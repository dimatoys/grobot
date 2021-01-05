#include "camera.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <time.h>
#include <librealsense2/rs.hpp>


class TPipeline {
	rs2::pipeline	pipeline;
	rs2::config		cfg;

	rs2::frameset frames;

public:

	int width;
	int height;

	TPipeline() {
		cfg.enable_stream(RS2_STREAM_DEPTH, RS2_FORMAT_Z16);
		pipeline.start(cfg);
	}

	~TPipeline() {
		pipeline.stop();
	}

	void newFrame() {
		frames = pipeline.wait_for_frames();
		rs2::depth_frame depth = frames.get_depth_frame();
		width = depth.get_width();
		height = depth.get_height();
	}

	void saveDepth() {

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

	float get_distance(int x, int y) {
		return frames.get_depth_frame().get_distance(x,y);
	}

};


TCamera::TCamera() {

	buffer_size = 640*480;
	buffer = new uint8_t[buffer_size];

	limit = 0.6;

	pipeline = new TPipeline();
}

TCamera::~TCamera() {
	delete (TPipeline*)pipeline;
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
	
	TPipeline* pl = (TPipeline*)pipeline;
	pl->newFrame();

	//pl->saveDepth();
}

void TCamera::process() {

	TPipeline* pl = (TPipeline*)pipeline;

	max = 0;
	min = limit;

	for (auto y = 0; y < pl->height; ++y) {
		for (auto x = 0; x < pl->width; ++x) {
			float d = pl->get_distance(x, y);
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
		uint8_t* data = new uint8_t[pl->width * pl->height * channels];
		auto p = data;
		for (auto y = 0; y < pl->height; ++y) {
			for (auto x = 0; x < pl->width; ++x) {
				float d = pl->get_distance(x, y);
				if (d > limit) {
					d = limit;
				}
				if ((d == 0) || ((d >= 0.26) && (d < 0.57))) {
					uint8_t color = d > 0 ? (uint8_t)(255 * (d - min) / interval) : 0;
					*p++ = color;
					*p++ = color;
					*p++ = color;
				} else {
					*p++ = 100;
					*p++ = 100;
					*p++ = 255;
				}
			}
		}

		stbi_write_jpg_to_func(
			[](void *context, void *compressed, int size) {
				((TCamera*)context)->writeBuffer(compressed, size);
			}, this, pl->width, pl->height, channels, data, 90);
		delete data;
	}
}
