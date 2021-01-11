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
	setServoValue(grobot, servo, (rservo->hardware_max - rservo->hardware_min) * angle / 1000 + rservo->hardware_min);
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
