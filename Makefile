BUILD_FLAGS = -lbcm2835 -Llib/librealsense2 -lrealsense2

all:: libgrobot.so


%.o : %.C
	g++ -g -Wall -std=c++11 -I/opt/vc/include -Ilib -Imodule -DRASPPI -DNDEBUG -fno-rtti -fPIC -c $< -o $@

%.so: module/grobot.o lib/pca9685.o module/camera.o module/regression.o
	g++ -shared -o $@ $^ $(BUILD_FLAGS)

test1: tests/test1.o lib/pca9685.o
	g++ -o $@ $^ $(BUILD_FLAGS)

test2: tests/test2.o module/camera.o module/regression.o
	g++ -o $@ $^ $(BUILD_FLAGS)

clean:
	rm test2 */*.o *.so
