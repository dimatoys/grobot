#include "camera.h"
#include "regression.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <time.h>
#include <stdlib.h>
#include <list>
#include <vector>

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

	reg_k = 10;
	reg_pr = new double[reg_k];
/*
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
*/

	reg_pr[0] = 358.500017400887;
	reg_pr[1] = 0.017839485073;
	reg_pr[2] = -0.534426058830;
	reg_pr[3] = -0.000006821775;
	reg_pr[4] =  0.000850294317;
	reg_pr[5] = -0.000065215744;
	reg_pr[6] =  0.000000007386;
	reg_pr[7] = -0.000001221031;
	reg_pr[8] =  0.000000035413;
	reg_pr[9] =  0.000000108109;


	reg_surface = NULL;
}

TCamera::~TCamera() {
	if (data != NULL) {
		delete data;
	}
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

#define AREA(x, y) (x >= 92) && (x + y >= 302) && (y-x >= 130-848) && ((y <= 340) || (x<= 750 )) && ((y <= 422) || (x <= 350) || (x >= 640))

void TCamera::calibrate(TDepth* depth) {

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

void TCamera::drawDepth(TDepth* depth) {

	allocate(depth->width, depth->height, 3);

	max = 0;
	min = limit;

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

struct TSlice {
	int	from_y;
	int	to_y;
	int	obj_id;

	std::list<TSlice*> refs;

	TSlice(int from_y, int to_y) {
		this->from_y   = from_y;
		this->to_y     = to_y;
		this->obj_id = -1;
	}

	TSlice(const TSlice& slice) {
		this->from_y = slice.from_y;
		this->to_y   = slice.to_y;
		this->refs   = slice.refs;
	}
};

struct TObject {
	int min_x;
	int max_x;
	int min_y;
	int max_y;

	std::list<int> refs;
	bool has_ref;

	TObject(int x, int min_y, int max_y) {
		min_x = x;
		max_x = x;
		this->min_y = min_y;
		this->max_y = max_y;
		has_ref = false;
	}

	void add(int x, int min_y, int max_y) {
		min_x = x;
		if (min_y < this->min_y ) {
			this->min_y = min_y;
		}
		if (max_y > this->max_y) {
			this->max_y = max_y;
		}
	}

	void addRef(int obj_id) {
		refs.push_back(obj_id);
	}
};

void TCamera::processInit(TDepth* depth) {
	int sheight = 434;
	
	if (reg_surface == NULL) {
		reg_surface = new int16_t[depth->width * sheight];
		auto r = reg_surface;
		auto w2 = depth->width / 2;
		auto h2 = depth->height / 2;
		for (int y = 0; y < sheight; ++y) {
			for (int x = 0; x < depth->width; ++x) {
				*r++ = (int16_t)Predict(x - w2, y - h2, reg_k, reg_pr);
			}
		}
	}
}

void TCamera::process(TDepth* depth) {

	int sheight = 434;

	int step = 10;
	int min_y = 20;
	int16_t min_h = 10;

	std::list<TSlice*> row1;
	std::list<TSlice*> row2;

	std::list<TSlice*>* current_row = &row1;
	std::list<TSlice*>* last_row    = &row2;

	std::vector<TObject*> objs;

	for (int x = depth->width - 1; x >= 0 ; x -= step) {
		int start_y = -1;
		int last_h;
		for (int y = sheight-1; y >=0; --y) {
			auto d = depth->get_distance_mm(x, y);
			if (d > 0) {
				auto h = reg_surface[x + y * depth->width] - (int16_t)d;
				if (h > min_h) {
					if (start_y < 0) {
						start_y = y;
					}
					last_h = y;
				} else {
					if (start_y >= 0) {
						if (start_y - last_h >= min_y) {
							TSlice* slice = new TSlice(start_y, last_h);
							current_row->push_back(slice);
							for(std::list<TSlice*>::const_iterator i = last_row->cbegin(); i != last_row->cend(); ++i) {
								if ((*i)->from_y > start_y ? (*i)->to_y < start_y : last_h < (*i)->from_y) {
									slice->refs.push_back(*i);
									if (slice->obj_id == -1) {
										slice->obj_id = (*i)->obj_id;
									} else {
										if (slice->obj_id != (*i)->obj_id) {
											if (slice->obj_id > (*i)->obj_id) {
												objs[slice->obj_id]->addRef((*i)->obj_id);
												objs[(*i)->obj_id]->has_ref = true;

											} else {
												objs[(*i)->obj_id]->addRef(slice->obj_id);
												objs[slice->obj_id]->has_ref = true;
												slice->obj_id = (*i)->obj_id;
											}
										}
									}
								}
							}

							if (slice->obj_id == -1) {
								slice->obj_id = objs.size();
								//printf("new obj=%d x=%d y=[%d..%d]\n", slice->obj_id, x, last_h, start_y);
								objs.push_back(new TObject(x, last_h, start_y));
								//for (auto a = last_h; a <= start_y; ++a) {
								//	data[(x + a * depth->width) * 3] = 255;
								//}
							} else {
								objs[slice->obj_id]->add(x, last_h, start_y);
								//printf("add obj=%d x=%d y=[%d..%d]\n", slice->obj_id, x, last_h, start_y);
								//for (int a = 0; a < step; ++a) {
								//	data[(x + a + slice->from_y * depth->width) * 3] = 255;
								//	data[(x + a + slice->to_y * depth->width) * 3] = 255;
								//}
								//for (auto a = last_h; a <= start_y; ++a) {
								//	data[(x + a * depth->width) * 3 + 1] = 255;
								//}
							}
						}
						start_y = -1;
					}
				}
			}
		}

		std::list<TSlice*>* tmp = last_row;
		last_row = current_row;
		current_row = tmp;

		for(std::list<TSlice*>::const_iterator i = current_row->cbegin(); i != current_row->cend(); ++i) {
			delete *i;
		}
		current_row->clear();
	}

	for(std::list<TSlice*>::const_iterator i = row1.cbegin(); i != row1.cend(); ++i) {
		delete *i;
	}
	for(std::list<TSlice*>::const_iterator i = row2.cbegin(); i != row2.cend(); ++i) {
		delete *i;
	}

	allocate(depth->width, depth->height, 3);

	for(int i = objs.size() - 1; i >= 0 ; --i) {
		TObject* obj = objs[i];
		for (std::list<int>::const_iterator ri = obj->refs.cbegin(); ri != obj->refs.cend(); ++ri) {
			TObject* ref_obj = objs[*ri];
			//printf("check ref=%d: x[%d..%d]-y[%d..%d]\n", *ri, ref_obj->min_x, ref_obj->max_x, ref_obj->min_y, ref_obj->max_y);
			if (ref_obj->min_x < obj->min_x) {
				obj->min_x = ref_obj->min_x;
			}
			if (ref_obj->max_x > obj->max_x) {
				obj->max_x = ref_obj->max_x;
			}
			if (ref_obj->min_y < obj->min_y) {
				obj->min_y = ref_obj->min_y;
			}
			if (ref_obj->max_y > obj->max_y) {
				obj->max_y = ref_obj->max_y;
			}
		}
		//printf("%d: x[%d..%d]-y[%d..%d] %d %s\n", i, obj->min_x, obj->max_x, obj->min_y, obj->max_y, (int)obj->refs.size(), obj->has_ref ? "CHILD" : "ROOT");

		if (!obj->has_ref) {
			for (int x = obj->min_x; x <= obj->max_x; ++x) {
				data[(x + obj->min_y * depth->width) * 3 + 2] = 255;
				data[(x + obj->max_y * depth->width) * 3 + 2] = 255;
			}
			for (int y = obj->min_y; y <= obj->max_y; ++y) {
				data[(obj->min_x + y * depth->width) * 3 + 2] = 255;
				data[(obj->max_x + y * depth->width) * 3 + 2] = 255;
			}
		}

		delete obj;
	}
}

double yToDistance(double* y, double* params) {
	auto yViewAngle = params[0];
	auto angle      = params[1];
	auto high       = params[2];
	auto curve      = params[4];

	double a = curve / 100;
	double b = 1 - a;
	double v = (a * *y + b) * *y;
	double d = high / cos((angle + yViewAngle * v / 2) * M_PI / 180);

	return d;
}


void TCamera::calibrate2(TDepth* depth) {
	uint8_t min_color[] = {100,100,255};
	for (int y = 0; y <depth->height;++y) {
		int min = 1000000;
		for (int x = 0; x < depth->width;++x) {
			auto d = depth->get_distance_mm(x, y);
			if (d > 0 && d < min) {
				min = d;
			}
		}
		for (int x = 0; x < depth->width;++x) {
			if (depth->get_distance_mm(x, y) == min) {
				set(x,y, min_color);
			}
		}
	}
/*
	double params[4] = {50, 45, 330, 0};
	
	double x[400];
	double y[400];
	int n = 0;
	for (int i = depth->height - 1; i >= depth->height / 2; i -= 1) {
		x[n] = 2.0 * (depth->height / 2.0 - i) / depth->height; // -1 ..  + 1
		y[n] = depth->get_distance_mm(depth->width / 2 , i);
		if (y[n] > 0) {
			++n;
		}
	}

	double d = BestFit(yToDistance, n, 1, x, y, 4, params, 0.1);

	for (int i = 0; i < n; ++i) {
		auto d = yToDistance(x + i, params);
		printf("%f: %f -> %f\n", x[i], y[i], d);
	}

	printf("d=%f n=%d view=%f angle=%f high=%f curve=%f\n", sqrt(d) / n, n, params[0], params[1], params[2], params[3]);
*/
}
