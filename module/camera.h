#include <cstdint>
#include <string>

class TDepth {

public:

	int width;
	int height;

	virtual void newFrame()=0;
	virtual float get_distance(int x, int y)=0;
	virtual uint16_t get_distance_mm(int x, int y)=0;
	void save();

	virtual ~TDepth() {};

};

class TDepthFile : public TDepth {
	std::string fileName;
	float*  data;
public:
	TDepthFile(std::string fileName) {
		this->fileName = fileName;
		data = NULL;
	}

	~TDepthFile();

	void newFrame();

	float get_distance(int x, int y) {
		return data[y * width + x];
	}

	uint16_t get_distance_mm(int x, int y) {
		return (uint16_t)(data[y * width + x] * 1000);
	}
};

class TCamera {

	void resetBuffer();
	void writeBuffer(const void* data, int size);

public:
	int buffer_size;

	uint8_t* data;
	int width;
	int height;
	int components;

	uint8_t* buffer;
	int buffer_ptr;

	uint16_t min;
	uint16_t max;
	uint16_t limit;

	int       reg_k;
	double*   reg_pr;
	int16_t* reg_surface;

	TCamera();
	~TCamera();

	void allocate(int newWidth, int newHeight, int newComponents);
	void set(int x, int y, uint8_t* color);

	void saveJpg();
	void drawDepth(TDepth* depth);

	void calibrate(TDepth* depth);
	void calibrate2(TDepth* depth);

	void processInit(TDepth* depth);
	void process(TDepth* depth);
};
