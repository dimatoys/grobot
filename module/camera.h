#include <cstdint>
#include <string>

class TDepth {

public:

	int width;
	int height;

	virtual void newFrame()=0;
	virtual float get_distance(int x, int y)=0;
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
};

class TCamera {
	TDepth* depth;

	void resetBuffer();
	void writeBuffer(const void* data, int size);
	int buffer_size;

public:
	uint8_t* buffer;
	int buffer_ptr;

	float min;
	float max;
	float limit;

	int     reg_k;
	double* reg_pr;
	double* reg_surface;

	TCamera(TDepth* depth);

	void scan();
	void showDepth();
	void calibrate();
	void process();
};
