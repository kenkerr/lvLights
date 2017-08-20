#ifndef MB_REGISTERS_H
#define MB_REGISTERS_H

class ModbusRegisters {

// Register Definitions
#define MB_MAX_HRS                  17

#define MB_CONTROLLER_MODE           0
#define MB_MANUAL_COMMAND            1
#define MB_TURN_ON_TIME              2          // minutes since midnight
#define MB_TURN_OFF_TIME             3          // minutes since midnight
#define MB_DUSK_OFF_DELAY            4          // minutes after dusk to turn off
#define MB_CONTROLLER_LONGITUDE      5          // HR[5:6] 0..360
#define MB_CONTROLLER_LATITUDE       7          // HR[7:8] 0..360
#define MB_CURRENT_TIME              9          // HR[9:10] seconds since epoch
#define MB_TIMEZONE                 11          // Hours from UTC [-12.. +12]
#define MB_IP_OCTET_1               12
#define MB_IP_OCTET_2               13
#define MB_IP_OCTET_3               14
#define MB_IP_OCTET_4               15
#define MB_CURRENT_STATE            16          // 1-OFF;

// Controller modes

#define CONTROLLER_MODE_MANUAL                  1
#define CONTROLLER_MODE_AUTO_ABS_TIME           2
#define CONTROLLER_MODE_AUTO_DUSK_TO_DAWN       3
#define CONTROLLER_MODE_AUTO_DUSK_TO_OFFSET     4

// MANUAL COMMANDS
#define MANUAL_COMMAND_OFF                      1
#define MANUAL_COMMAND_ON                       2
#define MANUAL_COMMAND_BLINK                    3

// Light states
#define CURRENT_STATE_OFF                       1
#define CURRENT_STATE_ON                        2

public:
    ModbusRegisters () ;
    ~ModbusRegisters() ;

    short getHR (int hr){
        return mbHR[hr];
    }
    int setHR (int hr, short value){
        mbHR[hr] = value;
    }
    int saveHR () ;

private:
    short mbHR[MB_MAX_HRS];

    void dumpHR ();

};
#endif
