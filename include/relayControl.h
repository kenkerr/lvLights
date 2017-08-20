#ifndef RELAYCONTROL_H
#define RELAYCONTROL_H
#include <string>

#include "LV_Lights.h"

#define IO_HEALTH_OK            0
#define IO_HEALTH_INIT_FAILED   1
#define RELAY_OFF               0                                       // AKA HIGH in wiringPi
#define RELAY_ON                1                                       // AKA LOW  in wiringPi

class RelayControl {

public:
	RelayControl();
	~RelayControl();

	int setRelayOn();
	int setRelayOff();
	int getRelayState();
    int getIoHealth() {
        return ioSubSystemHealth;
    };
    int setIoHealth(int health) {
        ioSubSystemHealth = health;
        return 0;
    };

	std::string getRelayStateDesc();

private:
	int relayState;
    int ioSubSystemHealth;
//	static const char *const relayStateDesc[] = {"OFF\n", "ON\n"};
//	static const char *const relayStateDesc[];
//	static const std::string relayStateDesc;
};
#endif
