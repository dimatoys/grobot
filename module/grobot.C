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

/*
	xs = {-1613.201879345892, 5.531028268229079, 0.39665757748397823, -0.005686617307062693, -0.0026999567290731316, 0.0016618452070674642, 2.2252732011741366e-06, 3.1853788392473506e-06, -4.5489920381840095e-07, -6.985220752334758e-07}
	ys = {1705.0840484817295, -2.6137918204190935, -2.562043025161781, 0.0007975344101505688, 0.0008025485407719204, 0.001704913353701013, 3.9723191781403237e-07, 9.653545544912878e-07, 2.252183635280757e-07, -4.6456834411302205e-07}

	xs= {-632.8593914066223, 2.6881031821752117, -3.8507928758370156, -0.003702986383015583, 0.007407363278528867, 0.007986006681403382, 2.934955440072012e-06, -3.5178800661100212e-06, -1.0996386491795207e-05, -4.540841833070183e-06, -8.356833991842709e-10, 9.98758227440592e-10, 3.5451617043590598e-09, 3.618175966007664e-09, 7.467511832541428e-10}
	ys= {5228.205575012951, -19.605950863335067, -9.69845698051337, 0.0309700716878597, 0.027354564865747277, 0.006804552524770781, -2.2983863555156765e-05, -3.1896956157015334e-05, -1.181587681275987e-05, -2.375268943577864e-06, 6.680902591739567e-09, 1.327254208439857e-08, 7.918313805661544e-09, 1.420311555221756e-09, 4.964498728112563e-10}

	xs= {-22139.407351698046, 146.02681244760615, 48.072475015170795, -0.3764678978298519, -0.28310607253249975, -0.03566673508250139, 0.00047688714462558197, 0.0005837119799575051, 0.00019186737826239412, 7.087101427285192e-06, -2.9598427437621617e-07, -5.124635920744782e-07, -2.8276511411788787e-07, -5.1908266033806285e-08, 4.181392340002441e-09, 7.212962455522759e-11, 1.646221606464974e-10, 1.2899280945626122e-10, 4.302211757718132e-11, 4.166894524861036e-12, -2.0684451688760808e-12}
	ys= {27318.307332947134, -160.23228380788146, -67.80022118178273, 0.3861133920566744, 0.32452939784740586, 0.0656688314214203, -0.00046735452833498683, -0.0005985275123963885, -0.00023688291585245347, -3.16554791661876e-05, 2.8207743849661305e-07, 4.885110168828541e-07, 2.974455233129142e-07, 7.257388610681703e-08, 8.433165930141292e-09, -6.763432968691822e-11, -1.4759860039683285e-10, -1.234756321422349e-10, -4.592228200633866e-11, -7.97613496287692e-12, -1.1673271894433831e-12}
*/
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


