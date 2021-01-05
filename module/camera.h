#include <librealsense2/rs.hpp>

class TCamera {
	rs2::pipeline	pipeline;
	rs2::config		cfg;

	void resetBuffer();
	void writeBuffer(const void* data, int size);
	int buffer_size;

	void saveDump(rs2::depth_frame depth);

public:
	int width;
	int height;

	uint8_t* buffer;
	int buffer_ptr;

	float min;
	float max;
	float limit;

	TCamera();
	~TCamera();

	void scan();
};
