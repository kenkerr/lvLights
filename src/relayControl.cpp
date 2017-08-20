#include "relayControl.h"
#include <wiringPi.h>

#define RELAY_GPIO27_PIN   2

// Constructor
RelayControl::RelayControl () {

    if (wiringPiSetup() == -1) {
        cout << "RelayControl> wiringPi initialization failed" << endl;

        // Indicate I/O subsystem problem
        ioSubSystemHealth = IO_HEALTH_INIT_FAILED;
        return;
    }
    
    ioSubSystemHealth = IO_HEALTH_OK;                                   // Set I/O health

    pinMode (RELAY_GPIO27_PIN, OUTPUT);                                 // Set realy pin to out

    relayState = RELAY_OFF;
    digitalWrite (RELAY_GPIO27_PIN, relayState);
    cout << "RelayControl> initialized relay to " << getRelayStateDesc() << endl;
};

std::string RelayControl::getRelayStateDesc() {
	if (relayState == RELAY_ON) {
		return ("ON");
	}
	else {
		return ("OFF");

	}
}
RelayControl::~RelayControl() {
	cout << "RelayControl> Executing destructor\n";
};


int RelayControl::setRelayOn() {
    cout << "RelayControl> Current relay state:\t\t\t" << getRelayStateDesc() << endl;

    relayState = RELAY_ON;
    digitalWrite (RELAY_GPIO27_PIN, relayState);

    cout << "RelayControl> New relay state:\t\t\t" << getRelayStateDesc() << endl;
    return relayState;
}


int RelayControl::setRelayOff() {
    cout << "RelayControl> Current relay state:\t\t\t" << getRelayStateDesc() << endl;

    relayState = RELAY_OFF;
    digitalWrite (RELAY_GPIO27_PIN, relayState);

    cout << "RelayControl> New relay state:\t\t\t" << getRelayStateDesc() << endl;

    return relayState;
}

int RelayControl::getRelayState() {
	cout << "RelayControl> Current relay state:\t\t\t" << getRelayStateDesc() << endl;
	return relayState;
}
