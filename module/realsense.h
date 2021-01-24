#include "camera.h"

#include <librealsense2/rs.hpp>

class TRealSense : public TDepth {
	rs2::pipeline	pipeline;
	rs2::config		cfg;

	rs2::frameset frames;
	uint16_t*     data;

public:

	TRealSense() {
		cfg.enable_stream(RS2_STREAM_DEPTH, RS2_FORMAT_Z16);
		//cfg.enable_stream(RS2_STREAM_DEPTH, 0, 848, 480, RS2_FORMAT_Z16, 30);
		pipeline.start(cfg);
	}

	~TRealSense() {
		pipeline.stop();
	}

	void newFrame() {
		frames = pipeline.wait_for_frames();
		rs2::depth_frame depth = frames.get_depth_frame();
		rs2::stream_profile sp = depth.get_profile();
		width = depth.get_width();
		height = depth.get_height();
		data = (uint16_t*)depth.get_data();

		printf("%s w=%d h=%d size=%d bits=%d bytes=%d stride=%d fps=%d\n",
			sp.stream_name().c_str(),
			width, height, depth.get_data_size(),
			depth.get_bits_per_pixel(),
			depth.get_bytes_per_pixel(),
			depth.get_stride_in_bytes(),
			sp.fps());
	}

	float get_distance(int x, int y) {
		//return frames.get_depth_frame().get_distance(x,y);
		return data[x + y * width] / (float)1000;
	}

	uint16_t get_distance_mm(int x, int y) {
		return data[x + y * width];
	}
};
