#include <cstdint>

class TCamera {
	void* pipeline;

	void resetBuffer();
	void writeBuffer(const void* data, int size);
	int buffer_size;

public:
	uint8_t* buffer;
	int buffer_ptr;

	float min;
	float max;
	float limit;

	TCamera();
	~TCamera();

	void scan();
	void process();
};
