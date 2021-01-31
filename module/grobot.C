#include "grobot.h"
#include "pca9685.h"
#include "realsense.h"
#include "regression.h"

#include <stdio.h>
#include <bcm2835.h>
#include <math.h>

int PIN_LEFT_GRIPPER = 23;
int PIN_RIGHT_GRIPPER = 24;

void ABtoPOS(double* x, double* y) {
    double R1 = 175;
    double R2 = 40;
    double R3 = 265;
    double R4 = 59;
    double a = M_PI * (x[0] - 500) / 1000;
    double b = M_PI * (x[1] - 500) / 1000;

    y[0] = R1 * sin(b) + R2 * sin(b - M_PI / 4) + R3 * sin(a + b - M_PI / 4) + R4 * sin(a + b - M_PI / 4 - M_PI / 2);
    y[1] = R1 * cos(b) + R2 * cos(b - M_PI / 4) + R3 * cos(a + b - M_PI / 4) + R4 * cos(a + b - M_PI / 4 - M_PI / 2);
}

void generateABPR(TGRobot* grobot, int k) {
	double ab[] = {	1000, 200,
					 800, 300,
					 900, 300,
					1000, 300,
					 300, 500,
					 500, 500,
					 700, 500,
					 900, 500,
					1000, 500,
					 100, 600,
					 300, 600,
					 500, 600,
					 700, 600,
					 900, 600,
					1000, 600,
					   0, 800,
					 100, 800,
					 300, 800,
					 500, 800,
					   0,1000,
					 100,1000};
	double pos[] = {
					-308,283,
					-331,324,
					-248,369,
					-166,383,
					-300,170,
					-253,354,
					 -97,459,
					  84,437,
					 151,389,
					-196,79,
					-213,274,
					 -96,425,
					  86,461,
					 245,371,
					 290,300,
					 -96,96,
					 -78,199,
					  47,347,
					 243,364,
					   5,135,
					  85,195
};
	int n = sizeof(ab) / sizeof(ab[0]) / 2;

	double* x = new double[n*2];
	for (int i = 0; i < n;++i) {
		x[i*2] = ab[i*2];
		x[i*2+1] = ab[i*2+1];
		Reverse(ABtoPOS,pos +i*2, 2, x + i*2, 2, 0.1);
	}
	// regression level, k[yn=1] 1..5 k[yn=2] 1,3,6,10,15
	grobot->surfacePosK = k;
	grobot->surfacePosPr = new double[k*2];

	GeneratePR(n, ab, 2, grobot->surfacePosK, x, 2, grobot->surfacePosPr);
/*
	for (int i = 0; i < n; ++i) {
		double xc[2];
		double y[2];
		Predict(ab + i*2, 2, grobot->surfacePosPr, grobot->surfacePosK, xc, 2);
		ABtoPOS(xc,y);
		printf("%f %f -> %f %f -> %f %f (%f %f -> %f %f) %f\n", ab[i*2], ab[i*2+1], xc[0], xc[1], y[0], y[1], x[i*2], x[i*2+1], pos[i*2], pos[i*2+1],
			sqrt((y[0] -pos[i*2]) * (y[0] -pos[i*2]) + (y[1] -pos[i*2+1]) * (y[1] -pos[i*2+1])));
	}
*/
	delete x;

	double abc[2];
	double xmin = 0;
	double xmax = 0;
	double ymin = 200;
	double ymax = 200;
	
	for (abc[0] = 0.0; abc[0] <= 1000.0; abc[0] += 1.0) {
		for (abc[1] = 0.0; abc[1] <=1000.0; abc[1] += 1.0) {
			double abm[2];
			double xy[2];
			Predict(abc, 2, grobot->surfacePosPr, grobot->surfacePosK, abm, 2);
			ABtoPOS(abm,xy);
			if (xy[0] < xmin) {
				xmin = xy[0];
			} else {
				if (xy[0] > xmax) {
					xmax = xy[0];
				}
			}
			if (xy[1] < ymin) {
				ymin = xy[1];
			} else {
				if (xy[1] > ymax) {
					ymax = xy[1];
				}
			}
		}
	}

}


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

	grobot->camera = new TCamera();
	grobot->realsense = new TRealSense();

	generateABPR(grobot, 6);


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

void setSurfacePosition(TGRobot* grobot, double x, double y) {
}

void depth(TGRobot* grobot) {
	TCamera* camera = (TCamera*)grobot->camera;
	TDepth* depth = (TDepth*)grobot->realsense;
	depth->newFrame();
	camera->drawDepth(depth);
	camera->saveJpg();
	grobot->picture.buffer = (char*)camera->buffer;
	grobot->picture.buffer_size = camera->buffer_ptr;
}

void calibrate(TGRobot* grobot) {
	TCamera* camera = (TCamera*)grobot->camera;
	TDepth* depth = (TDepth*)grobot->realsense;
	depth->newFrame();
	camera->drawDepth(depth);
	camera->calibrate2(depth);
	camera->saveJpg();
	grobot->picture.buffer = (char*)camera->buffer;
	grobot->picture.buffer_size = camera->buffer_ptr;
}


void scan(TGRobot* grobot) {
	TCamera* camera = (TCamera*)grobot->camera;
	TDepth* depth = (TDepth*)grobot->realsense;
	depth->newFrame();
	camera->drawDepth(depth);
	camera->processInit(depth);
	camera->process(depth);
	camera->saveJpg();
	grobot->picture.buffer = (char*)camera->buffer;
	grobot->picture.buffer_size = camera->buffer_ptr;
}

/*
A,B
1000, 200: 136,139 500:216,128 1000:298,169 -308,283
 800, 300: 116,110 500:196, 88 1000:291,107 -331,324
 900, 300: 215, 86 500:283, 80 1000:364,119 -248,369
1000, 300: 289, 83 500:359, 90 1000:417,143 -166,383
 300, 500:  66,231 500:131,151 1000:234,106 -300,170
 500, 500: 200, 87 500:282, 59 1000:374, 61 -253,354
 700, 500: 365, 35 500:439, 36 1000:511, 73 -97,459
 900, 500: 526, 51 500:583, 79 1000:627,150 84,437
1000, 500: 586, 82 500:641,122 1000:658,205 151,389
 100, 600: 151,348 500:188,240 1000:267,158 -196,79
 300, 600: 216,137 500:285, 89 1000:381, 71 -213,274
 500, 600: 361, 46 500:438, 38 1000:514, 64 -96,425
 700, 600: 519, 37 500:587, 55 1000:658,111 86,461
 900, 600: 681,100 500:739,144 1000:760,229 245,371
1000, 600: 757,152 500:797,214 1000:785,305 290,300
   0, 800: 309,336 500:366,240 1000:452,173 -96,96
 100, 800: 353,209 500:421,155 1000:523,134 -78,199
 300, 800: 503, 95 500:580, 93 1000:688,130 47,347
 500, 800: 682, 95 500:764,126 1000:847,205 243,364
   0,1000: 452,288 500:547,250 1000:672,257 5,135
 100,1000: 559,222 500:656,218 1000:791,273 85,195

(1000, 200) [1097.6999999999384, 95.10000000000596]
(800, 300) [929.0000000000293, 203.09999999999303]
(900, 300) [1013.2000000000257, 219.5999999999921]
(1000, 300) [1100.0999999999362, 226.99999999999167]
(300, 500) [284.4999999999965, 496.4999999999992]
(500, 500) [504.9000000000011, 501.90000000000043]
(700, 500) [661.5999999999913, 542.8000000000097]
(900, 500) [1011.5000000000254, 469.39999999999304]
(1000, 500) [1100.4999999999359, 474.79999999999427]
(100, 600) [55.20000000000192, 617.800000000004]
(300, 600) [288.8999999999975, 619.3000000000044]
(500, 600) [507.0000000000016, 627.5000000000063]
(700, 600) [660.199999999991, 668.6000000000156]
(900, 600) [1013.1000000000257, 593.7999999999986]
(1000, 600) [1100.4999999999359, 601.5000000000003]
(0, 800) [-62.70000000000062, 869.8000000000159]
(100, 800) [59.000000000001975, 873.7000000000168]
(300, 800) [295.1999999999989, 868.5000000000156]
(500, 800) [513.5000000000031, 881.8000000000186]
(0, 1000) [-63.80000000000064, 1132.9999999999063]
(100, 1000) [57.40000000000195, 1124.7999999999138]

def getPos(data):
    R1 = 175
    R2 = 40
    R3 = 265
    R4 = 59

    (A,B) = data
    
    a = pi * (A - 500) / 1000
    b = pi * (B - 500) / 1000
    
    return (R1 * sin(b) + R2 * sin(b - pi / 4) + R3 * sin(a + b - pi / 4) + R4 * sin(a + b - pi / 4 - pi / 2),
            R1 * cos(b) + R2 * cos(b - pi / 4) + R3 * cos(a + b - pi / 4) + R4 * cos(a + b - pi / 4 - pi / 2))

*/


