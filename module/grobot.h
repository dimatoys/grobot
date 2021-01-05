#define SERVO_L	0
#define SERVO_G	1
#define SERVO_A	2
#define SERVO_B	3


extern "C" {

struct TServo {
	int angle;			// -1000..1000
	int hardware_min;
	int hardware_max;
};

struct TPicture {
	int		buffer_size;
	char*	buffer;
};

struct TGRobot {
	void*	pca9685;
	void*	camera;
	int		numServos;
	TServo	servos[16];
	TPicture picture;
};

void init(TGRobot* grobot);
void cloase(TGRobot* grobot);
unsigned int readSensors(TGRobot* grobot);
void setServoValue(TGRobot* grobot, int servo, int value);
void setServoAngle(TGRobot* grobot, int servo, int angle);
void scan(TGRobot* grobot);

}