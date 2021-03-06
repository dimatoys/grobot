#include <cstdint>
#include <string>

class TDepth {

public:

	int width;
	int height;

	virtual void newFrame()=0;
	virtual float get_distance(int x, int y)=0;
	virtual uint16_t get_distance_mm(int x, int y)=0;
	void save(const char* fileName);
	void saveJson(const char* fileName);

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

const int MAX_OBJECTS = 100;

typedef uint8_t CELL_STATUS;
const CELL_STATUS CELL_UNDEFINED = 0;
const CELL_STATUS CELL_BACKGROUND = 1;
const CELL_STATUS CELL_NOT_CONFIDENT = 2;
const CELL_STATUS CELL_OBJECT = 3;
const CELL_STATUS CELL_SMALL_OBJECT = 4;

struct TObject {
	int x;
	int y;
	int rx;
	int ry;
	int cnt;
};

struct TCellFrontier{
	int x;
	int y;
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

	uint16_t limit;

	double* pr_a;
	double* pr_b;

	int cellSize;
	int minObjH;
	int minObjSize;
	CELL_STATUS* cell_cache;
	int cell_width;
	int cell_height;
	TCellFrontier* cell_frontier;
	int frontier_size;

	TObject objects[MAX_OBJECTS];
	int numObjects;

	TCamera();
	~TCamera();

	void allocate(int newWidth, int newHeight, int newComponents);
	void set(int x, int y, uint8_t* color);
	void set(int x0, int y0, int width, int height, uint8_t* color);
	void oval(int x, int y, int rx, int ry, uint8_t* color);

	void saveJpg();
	void drawDepth(TDepth* depth);

	void calibrate4(TDepth* depth);

	void initCache(TDepth* depth);
	CELL_STATUS isCellObject(TDepth* depth, int x0, int y0);
	void checkNewCell(TDepth* depth, int x, int y, int& cnt);
	bool processFrontier(TDepth* depth, TCellFrontier& cf, int& cnt);
	bool extractOvalObject(TDepth* depth, int x0, int y0, TObject* obj);

	void process4(TDepth* depth);
	void visualize4();

	bool extractLineObjects(TDepth* depth, int x0, int y0);
	void process5(TDepth* depth);
	void visualize5();

};
