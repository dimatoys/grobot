#include <bcm2835.h>
#include <stdio.h>

#include <iostream>
using namespace std;

#include "pca9685.h"

/*
https://www.airspayce.com/mikem/bcm2835/index.html

 */

 
// Input on RPi pin GPIO 23
//#define PIN RPI_GPIO_P1_23
 
//const int PIN = RPI_BPLUS_GPIO_J8_21;
 
const uint8_t MOTOR_GRAB = 1;		// 163 .. [430 - 500?] .. 1055
const uint8_t MOTOR_LIFT = 0;		// 164 .. [860] ..  1066
const uint8_t MOTOR_HAND = 2;		// 163 .. (360,620!) .. 1056
const uint8_t MOTOR_SHOULDER = 3;	// 164 .. 1060

void printAll() {
	for (int  PIN = 2; PIN <= 27; ++PIN) {
		// Set RPI pin P1-23 to be an input
		bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_INPT);
		//  with a pullup
		bcm2835_gpio_set_pud(PIN, BCM2835_GPIO_PUD_UP);
	}
 
    while (1)
    {
        // Read some data
        for (int  PIN = 2; PIN <= 27; ++PIN) {
			uint8_t value = bcm2835_gpio_lev(PIN);
			printf("read from pin %d: %d\n", (int)PIN, value);
		}
        
        printf("\n");
        delay(100);
    }
}

void readGripper() {
	int PIN1 = 23;
	int PIN2 = 24;
	
	bcm2835_gpio_fsel(PIN1, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_set_pud(PIN1, BCM2835_GPIO_PUD_UP);
 
	bcm2835_gpio_fsel(PIN2, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_set_pud(PIN2, BCM2835_GPIO_PUD_UP);


    int n = 0;
    while (1)
    {
		printf("%d read %d %d\n", ++n, (int)bcm2835_gpio_lev(PIN1), (int)bcm2835_gpio_lev(PIN2));
        delay(100);
    }
}

void controlMotor(uint8_t motor) {
	PCA9685* pca9685 = new PCA9685();
	pca9685->SetFrequency(100);
	
	while(1) {
	
		uint16_t i;
		cout << "value: ";
		cin >> i;
	
		pca9685->Write(motor, i);
	}
}

void grab() {
	
	int PIN1 = 23;
	int PIN2 = 24;
	
	bcm2835_gpio_fsel(PIN1, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_set_pud(PIN1, BCM2835_GPIO_PUD_UP);
 
	bcm2835_gpio_fsel(PIN2, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_set_pud(PIN2, BCM2835_GPIO_PUD_UP);

	PCA9685* pca9685 = new PCA9685();
	pca9685->SetFrequency(100);

	pca9685->Write((uint8_t)1, 550);
	
	delay(2000);
	
	for (uint16_t v = 540; v >= 430; v -=10) {
		pca9685->Write((uint8_t)1, v);
        delay(100);
        auto s1 = bcm2835_gpio_lev(PIN1);
        auto s2 = bcm2835_gpio_lev(PIN2);
		printf("%d read %d %d\n", (int)v, (int)s1, (int)s2);
		if ((s1 == 0) && (s2 == 0)) {
			printf("hold\n");
			break;
		}
	}
	
}


int main(int argc, char **argv)
{
 
    if (!bcm2835_init())
        return 1;
        
    //printAll();
	//readGripper();
 
	controlMotor(MOTOR_LIFT);
	
	//grab();
 
    bcm2835_close();
    return 0;
}
