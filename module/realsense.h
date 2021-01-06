#include "camera.h"

#include <librealsense2/rs.hpp>

class TRealSense : public TDepth {
	rs2::pipeline	pipeline;
	rs2::config		cfg;

	rs2::frameset frames;

public:

	TRealSense() {
		cfg.enable_stream(RS2_STREAM_DEPTH, RS2_FORMAT_Z16);
		pipeline.start(cfg);
	}

	~TRealSense() {
		pipeline.stop();
	}

	void newFrame() {
		frames = pipeline.wait_for_frames();
		rs2::depth_frame depth = frames.get_depth_frame();
		width = depth.get_width();
		height = depth.get_height();
	}

	float get_distance(int x, int y) {
		return frames.get_depth_frame().get_distance(x,y);
	}

};
