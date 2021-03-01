#include "camera.h"
#include "regression.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <time.h>
#include <stdlib.h>
#include <list>
#include <vector>
#include <algorithm>

void TDepth::save(const char* fileName) {

	char fileNameBuffer[100];
	if (fileName == NULL) {
		sprintf(fileNameBuffer,"depth%lu.csv", time(NULL));
		fileName = fileNameBuffer;
	}
	FILE* f = fopen(fileName, "w");
	for (auto y = 0; y < height; ++y) {
		const char* c = "";
		for (auto x = 0; x < width; ++x) {
			float d = get_distance(x, y);
			fprintf(f, "%s%f",c,d);
			c = ", ";
		}
		fputc('\n', f);
	}
	fclose(f);
}

void TDepth::saveJson(const char* fileName) {

	char fileNameBuffer[100];
	if (fileName == NULL) {
		sprintf(fileNameBuffer,"depth%lu.csv", time(NULL));
		fileName = fileNameBuffer;
	}
	FILE* f = fopen(fileName, "w");
	char ct = '[';
	for (auto y = 0; y < height; ++y) {
		fputc(ct,f);
		char c = '[';
		for (auto x = 0; x < width; ++x) {
			int d = get_distance_mm(x, y);
			fprintf(f, "%c%d",c,d);
			c = ',';
		}
		fputc(']', f);
		ct = ',';
	}
	fputc(']', f);
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

TCamera::TCamera() {

	buffer_size = 640*480;
	buffer = new uint8_t[buffer_size];
	data = NULL;

	limit = 600;

	pr_a = NULL;
	pr_b = NULL;
	cellSize = 10;
	minObjH = 10.0;
	minObjSize = 5;
	cell_cache = NULL;
}

TCamera::~TCamera() {
	if (data != NULL) {
		delete data;
	}
	//pr_a, pr_b, cell_cache
}

void TCamera::allocate(int newWidth, int newHeight, int newComponents) {
	if (data != NULL) {
		if ((width != newWidth) || (height != newHeight) || (components != newComponents)) {
			delete data;
		} else {
			return;
		}
	}
	width = newWidth;
	height = newHeight;
	components = newComponents;
	data = new uint8_t[width*height*components];
}

void TCamera::set(int x, int y, uint8_t* color) {
	memcpy(data + (x + width * y) * components, color, components);
}

void TCamera::set(int x0, int y0, int w, int h, uint8_t* color) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			set(x0 + x, y0 + y, color);
		}
	}

}

void TCamera::oval(int x, int y, int rx, int ry, uint8_t* color) {
	int x0 = x - rx;
	if (x0 < 0) {
		x0 = 0;
	}
	int x1 = x + rx;
	if (x1 >= width) {
		x1 = width  -1;
	}
	
	int r2 = rx * ry;
	
	for (int xi = x0; xi <= x1; ++xi) {
		int dy = (int)sqrt(r2 - (xi - x) * (xi - x));
		int yi = y - dy;
		if (yi >= 0) {
			set(xi, yi, color);
		}
		yi = y + dy;
		if (yi < height) {
			set(xi, yi, color);
		}
	}
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

void TCamera::drawDepth(TDepth* depth) {

	allocate(depth->width, depth->height, 3);

	uint16_t max = 0;
	uint16_t min = limit;

	for (int y = 0; y < depth->height; ++y) {
		for (int x = 0; x < depth->width; ++x) {
			auto d = depth->get_distance_mm(x, y);
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

	if (interval > 0) {
		auto p = data;
		for (auto y = 0; y < depth->height; ++y) {
			for (auto x = 0; x < depth->width; ++x) {
				auto d = depth->get_distance_mm(x, y);
				if (d > limit) {
					d = limit;
				}
				uint8_t color= d > 0 ? (uint8_t)(255 * (d - min) / interval) : 0;
				*p++ = color;
				*p++ = color;
				*p++ = color;
			}
		}
	}
}

void TCamera::saveJpg() {
	resetBuffer();
	stbi_write_jpg_to_func(
		[](void *context, void *compressed, int size) {
			((TCamera*)context)->writeBuffer(compressed, size);
		}, this, width, height, components, data, 90);
}

void TCamera::calibrate4(TDepth* depth) {
	if (pr_a == NULL) {
		pr_a = new double[depth->height];
		pr_b = new double[depth->height];
	}
	
	
	
	for (int y = 0; y < depth->height; ++y) {
		double sxy = 0;
		double sx  = 0;
		double sy  = 0;
		double sx2 = 0;

		int n = 0;
		for(int x = 0; x < depth->width; ++x) {
			auto d = depth->get_distance_mm(x,y);
			if (d > 0 && d < limit) {
				sxy += x * d;
				sx  += x;
				sy += d;
				sx2 += x * x;
				++n;
			}
		}

		pr_a[y] = (n * sxy - sx * sy) / (n * sx2 - sx * sx);
		pr_b[y] = (sy - pr_a[y] * sx) / n;
		//printf("%3d: [%10f,%10f]\n", y, pr_b[y], pr_a[y]);
	}
}

CELL_STATUS TCamera::isCellObject(TDepth* depth, int x0, int y0) {

	int n = 0;
	int c = 0;
	
	int x1 = x0 + cellSize;
	int y1 = y0 + cellSize;
	for (int y = y0; y < y1; ++y) {
		for(int x = x0; x < x1; ++x) {
			auto d = depth->get_distance_mm(x,y);
			if (d > 0 && d < limit) {
				++c;
				if (pr_b[y] + pr_a[y] * x - d > minObjH ) {
					++n;
				}
			}
		}
	}
	int cellPixels = cellSize * cellSize;
	
	return c >= 0.5 * cellPixels ?
		(n >= 0.8 * c ? CELL_OBJECT : CELL_BACKGROUND) :
			CELL_NOT_CONFIDENT;
}

struct TCellFrontier{
	int x;
	int y;
};

void TCamera::extractObject(TDepth* depth, int x0, int y0, TObject* obj) {
	int cnt  = 1;
	int minX = cell_width;
	int maxX = -1;
	int minY = cell_height;
	int maxY = -1;

	TCellFrontier* cell_frontier = new TCellFrontier[cell_width * cell_height];
	int frontier_size = 1;
	cell_frontier[0] = {x0, y0};
	
	while(frontier_size > 0) {
		TCellFrontier cf = cell_frontier[--frontier_size];
		if (cf.x < minX) {
			minX = cf.x;
		}
		if (cf.x > maxX) {
			maxX = cf.x;
		}
		if (cf.y < minY) {
			minY = cf.y;
		}
		if (cf.y > maxY) {
			maxY = cf.y;
		}

		int newX = cf.x - 1;
		if (newX >= 0) {
			CELL_STATUS c = cell_cache[cell_width*cf.y + newX];
			if (c == CELL_UNDEFINED) {
				c = isCellObject(depth, newX * cellSize, cf.y * cellSize);
				cell_cache[cell_width*cf.y + newX] = c;
				if (c == CELL_OBJECT) {
					cell_frontier[frontier_size++] = {newX, cf.y};
					++cnt;
				}
			}
		}
		newX = cf.x + 1;
		if (newX < cell_width) {
			CELL_STATUS c = cell_cache[cell_width*cf.y + newX];
			if (c == CELL_UNDEFINED) {
				c = isCellObject(depth, newX * cellSize, cf.y * cellSize);
				cell_cache[cell_width*cf.y + newX] = c;
				if (c == CELL_OBJECT) {
					cell_frontier[frontier_size++] = {newX, cf.y};
					++cnt;
				}
			}
		}
		int newY = cf.y - 1;
		if (newY >= 0) {
			CELL_STATUS c = cell_cache[cell_width*newY + cf.x];
			if (c == CELL_UNDEFINED) {
				c = isCellObject(depth, cf.x * cellSize, newY * cellSize);
				cell_cache[cell_width*newY + cf.x] = c;
				if (c == CELL_OBJECT) {
					cell_frontier[frontier_size++] = {cf.x, newY};
					++cnt;
				}
			}
		}
		newY = cf.y + 1;
		if (newY < cell_height) {
			CELL_STATUS c = cell_cache[cell_width*newY + cf.x];
			if (c == CELL_UNDEFINED) {
				c = isCellObject(depth, cf.x * cellSize, newY * cellSize);
				cell_cache[cell_width*newY + cf.x] = c;
				if (c == CELL_OBJECT) {
					cell_frontier[frontier_size++] = {cf.x, newY};
					++cnt;
				}
			}
		}
	}
	
	delete cell_frontier;
	obj->x = (minX + maxX) * cellSize / 2 + cellSize / 2;
	obj->y = (minY + maxY) * cellSize / 2 + cellSize / 2;
	obj->rx = (maxX - minX) * cellSize / 2;
	obj->ry = (maxY - minY) * cellSize / 2;
	obj->cnt = cnt;
}

void TCamera::process4(TDepth* depth) {
	if (pr_a == NULL) {
		calibrate4(depth);
	}

	if (cell_cache == NULL) {
		cell_width = depth->width / cellSize - 1;
		cell_height = depth->height / cellSize - 1;
		cell_cache = new CELL_STATUS[cell_width * cell_height];
	}

	memset(cell_cache,CELL_UNDEFINED,cell_width * cell_height);

	numObjects = 0;

	for (int y0 = 0; y0 < cell_height; ++y0) {
		for(int x0 = 0; x0 < cell_width; ++x0) {
			CELL_STATUS c = cell_cache[cell_width*y0 + x0];
			if (c == CELL_UNDEFINED) {
				c = isCellObject(depth, x0 * cellSize, y0 * cellSize);
				cell_cache[cell_width*y0 + x0] = c;
				if (c == CELL_OBJECT) {
					extractObject(depth, x0, y0, objects + numObjects);
					if (objects[numObjects].cnt >= minObjSize) {
						if(++numObjects ==MAX_OBJECTS) {
							return;
						}
					}
				}
			}
		}
	}
}

void TCamera::visualize4() {

	uint8_t cellBackground[] = {80, 80, 255};
	uint8_t cellObject[] = {255, 80, 80};
	uint8_t objColor[] = {80,255,80};
	
	for (int y0 = 0; y0 < cell_height; ++y0) {
		for(int x0 = 0; x0 < cell_width; ++x0) {
			switch(cell_cache[cell_width*y0 + x0]){
				case CELL_BACKGROUND:
					set(x0 * cellSize + cellSize / 2 - 1,
					    y0 * cellSize + cellSize / 2 - 1,
					    2,
					    2,
					    cellBackground);
					break;
				case CELL_OBJECT:
					set(x0 * cellSize + cellSize / 2 - 1,
					    y0 * cellSize + cellSize / 2 - 1,
					    2,
					    2,
					    cellObject);
					break;
			}
		}
	}

	for (int i = 0; i < numObjects; ++i) {
		oval(objects[i].x, objects[i].y, objects[i].rx, objects[i].ry, objColor);
	}
}
