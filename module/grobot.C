#include "grobot.h"
#include "pca9685.h"
#include "realsense.h"

#include <stdio.h>
#include <bcm2835.h>

int PIN_LEFT_GRIPPER = 23;
int PIN_RIGHT_GRIPPER = 24;

void init(TGRobot* grobot) {
	printf("init\n");
	if (!bcm2835_init()) {
		printf("Error init bcm2835\n");
        return;
    }
    
    grobot->lowMode = 0;

	grobot->numServos = 4;
	grobot->servos[SERVO_L].angle = 0;
	grobot->servos[SERVO_L].hardware_min = 400;
	grobot->servos[SERVO_L].hardware_max = 865;
	grobot->servos[SERVO_G].angle = 950;
	grobot->servos[SERVO_G].hardware_min = 440;
	grobot->servos[SERVO_G].hardware_max = 630;
	grobot->servos[SERVO_A].angle = 100;
	grobot->servos[SERVO_A].hardware_min = 320;
	grobot->servos[SERVO_A].hardware_max = 950;
	grobot->servos[SERVO_B].angle = 850;
	grobot->servos[SERVO_B].hardware_min = 250;
	grobot->servos[SERVO_B].hardware_max = 950;

	// init servos
	PCA9685* pca9685 = new PCA9685();
	pca9685->SetFrequency(100);
	grobot->pca9685 = pca9685;

	// init gripper
	bcm2835_gpio_fsel(PIN_LEFT_GRIPPER, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_set_pud(PIN_LEFT_GRIPPER, BCM2835_GPIO_PUD_UP);
 
	bcm2835_gpio_fsel(PIN_RIGHT_GRIPPER, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_set_pud(PIN_RIGHT_GRIPPER, BCM2835_GPIO_PUD_UP);

	grobot->camera = new TCamera(new TRealSense());

}

void close(TGRobot* grobot) {
	bcm2835_close();
	delete (TCamera*)grobot->camera;
}

unsigned int readSensors(TGRobot* grobot) {
	uint8_t left = bcm2835_gpio_lev(PIN_LEFT_GRIPPER) & 1;
	uint8_t right = bcm2835_gpio_lev(PIN_RIGHT_GRIPPER) & 1;

	return left + (right << 1);
}

void setServoValue(TGRobot* grobot, int servo, int value) {
	((PCA9685*)grobot->pca9685)->Write((uint8_t)servo, (uint16_t)value);
}

void setServoAngle(TGRobot* grobot, int servo, int angle) {
	TServo* rservo = &grobot->servos[servo];
	rservo->angle = angle;
	int value = (rservo->hardware_max - rservo->hardware_min) * angle / 1000 + rservo->hardware_min;
	setServoValue(grobot, servo, value);

	if ((servo == SERVO_A) && (grobot->lowMode > 0)) {
		int angleL = 0.0001 * angle * angle - 0.17 * angle + 1000;
		printf("LOW MODE: A=%d L=%d\n", angle, angleL);
		setServoAngle(grobot, SERVO_L, angleL);
	}
}

void depth(TGRobot* grobot) {
	TCamera* camera = (TCamera*)grobot->camera;
	camera->scan();
	uint8_t* data = new uint8_t[camera->depth->width * camera->depth->height * 3];
	camera->drawDepth(data);
	camera->saveJpg(data);
	delete data;
	grobot->picture.buffer = (char*)camera->buffer;
	grobot->picture.buffer_size = camera->buffer_ptr;
}

void scan(TGRobot* grobot) {
	TCamera* camera = (TCamera*)grobot->camera;
	camera->scan();
	uint8_t* data = new uint8_t[camera->depth->width * camera->depth->height * 3];
	camera->drawDepth(data);
	camera->process(data);
	camera->saveJpg(data);
	delete data;
	grobot->picture.buffer = (char*)camera->buffer;
	grobot->picture.buffer_size = camera->buffer_ptr;
}

/*
A,B
1000, 200: 136,139 500:216,128 1000:298,169
 800, 300: 116,110 500:196, 88 1000:291,107
 900, 300: 215, 86 500:283, 80 1000:364,119
1000, 300: 289, 83 500:359, 90 1000:417,143
 300, 500:  66,231 500:131,151 1000:234,106
 500, 500: 200, 87 500:282, 59 1000:374, 61
 700, 500: 365, 35 500:439, 36 1000:511, 73
 900, 500: 526, 51 500:583, 79 1000:627,150
1000, 500: 586, 82 500:641,122 1000:658,205
 100, 600: 151,348 500:188,240 1000:267,158
 300, 600: 216,137 500:285, 89 1000:381, 71
 500, 600: 361, 46 500:438, 38 1000:514, 64
 700, 600: 519, 37 500:587, 55 1000:658,111
 900, 600: 681,100 500:739,144 1000:760,229
1000, 600: 757,152 500:797,214 1000:785,305
   0, 800: 309,336 500:366,240 1000:452,173
 100, 800: 353,209 500:421,155 1000:523,134
 300, 800: 503, 95 500:580, 93 1000:688,130
 500, 800: 682, 95 500:764,126 1000:847,205
   0,1000: 452,288 500:547,250 1000:672,257
 100,1000: 559,222 500:656,218 1000:791,273
*/


