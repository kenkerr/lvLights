#include "LV_Lights.h"
#include <iostream>
#include <fstream>
#include "modbusTransaction.h"
#include "modbusRegisters.h"

// Constructor
ModbusRegisters::ModbusRegisters () {

    streampos   filesize;                                   

    cout << "ModbusRegisters> initializing..." << endl ;

    // Open the holding registers file (if it exists)
    ifstream hrFile ("/var/lib/lightController/holdingRegisters.bin", ios::in|ios::binary|ios::ate);
    if (hrFile.is_open()) {
        hrFile.seekg (0, ios::beg);
        hrFile.read ( (char *) mbHR, MB_MAX_HRS*2);
        hrFile.close();

    // If the file is not open (probably because it doesn't exist, seed the registers with zeros
    } else {
        
        for (int i=0; i<MB_MAX_HRS; i++) {
            mbHR[i] = 0;
        }
    }

    // Dump out important regs for debug
    dumpHR();
};

ModbusRegisters::~ModbusRegisters() {
    cout << "ModbusRegisters> Executing destructor\n";
};

int ModbusRegisters::saveHR () {

    ofstream hrFile ("/var/lib/lightController/holdingRegisters.bin", ios::out|ios::binary|ios::trunc);
    if (hrFile.is_open()) {
        
        hrFile.write ( (char *) mbHR, MB_MAX_HRS*2);
        hrFile.close();

        // Dump out important regs for debug
        dumpHR();

    // If the file is not open (probably because it doesn't exist, seed the registers with zeros
    } else {

        return 1;
    }
}

void ModbusRegisters::dumpHR () {

    cout << "=======================================================" << endl;

    if (mbHR[MB_CONTROLLER_MODE] == CONTROLLER_MODE_MANUAL) {
        cout << "Controller Mode:   Manual          Command: " << mbHR[MB_MANUAL_COMMAND] << endl;
    } else {
        cout << "Controller Mode:   Auto            subMode: " << mbHR[MB_CONTROLLER_MODE] << endl;
        cout << "On Time:          "<< mbHR[MB_TURN_ON_TIME] << endl;
        cout << "Off Time:         "<< mbHR[MB_TURN_OFF_TIME] << endl;
        cout << "Dusk Delay:       "<< mbHR[MB_DUSK_OFF_DELAY] << endl;
    }
    

    cout << "Current State:    " << mbHR[MB_CURRENT_STATE] << endl;

    cout << "=======================================================" << endl;
}
