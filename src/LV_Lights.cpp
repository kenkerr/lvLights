//
// LV_Lights.cpp : Defines the entry point for the console application.
//
#include "LvLightController.h"
#include "relayControl.h"
#include "nwComms.h"
#include "dawnDusk.h"

#define PROC_SUCCESS        0
#define PROC_ERROR          1

#define NO_ACTION_REQUIRED  0
#define TURN_LIGHTS_OFF     1
#define TURN_LIGHTS_ON      2

// LvLightController Constructor
LvLightController::LvLightController() {

}

// LvLightController destructor
LvLightController::~LvLightController() {

}

//==================================================================
//  Handler for modbus read requests
//==================================================================
int processRead (ModbusTransaction *req, char *resp, ModbusRegisters *mb){

    int         stAddr;

    short       nRegs;
    short       reqRegs[MB_MAX_HRS];
    short       val;

    cout << "LvLightController/processRead> processing read request" << endl;

    // Pre-load as much of the response buffer as we can
    val = htons (req->getTransID());
    memcpy ( (void *) &(resp[0]), (const void *) &val, sizeof (val) );
    
    val = htons (req->getProtoID());
    memcpy ( (void *) &(resp[2]), (const void *) &val, sizeof (val) );

    val = 3;                                                            // assume error => # of bytes to follow = 3
    val = htons (val);
    memcpy ( (void *) &(resp[4]), (const void *) &val, sizeof (val) );
    
    resp[6] = req->getUnitID();
    resp[7] = req->getFC();


    stAddr = req->getStAddr();
    nRegs  = req->getNRegs();

    // Check that requested starting address and length are valid
    if (stAddr < 0 || stAddr > MB_MAX_HRS) {
        resp[7] |= 0x80;
        resp[8] = MB_ILLEGAL_ADDRESS;
        return 9;
    }
        
    if ( (stAddr+nRegs) > MB_MAX_HRS) {
        resp[7] |= 0x80;
        resp[8] = MB_ILLEGAL_ADDRESS;
        return 9;
    }

        
    // correct the # of bytes-to-follow
    val  = 3 + (nRegs * 2);                                                 
    val  = htons (val);
    memcpy ( (void *) &(resp[4]), (const void *) &val, sizeof (val) );
    cout << "processRead> # bytes to follow = " << val << " " << resp[4] << " " << resp[5]<<endl;


    // Retrieve the registers and load them in the return buffer
    resp[8] = nRegs*2;
    for (int i=stAddr; i<(stAddr+nRegs); i++) {
        reqRegs[i] = htons (mb->getHR(i));
    }

    memcpy ( (void *) &(resp[9]), (const void *) &(reqRegs[stAddr]), nRegs*2);
        

    // Calculate the # of bytes returned
    val = 9 + nRegs*2;
    return val;

}

//==================================================================
// Handler for modbus write requests
//==================================================================
int processWrite (ModbusTransaction *req, char *resp, ModbusRegisters *mb, RelayControl *rc, DawnDusk *dd) {

    int         hrChanged[MAX_TRANSACTION_HRS];
    int         iVal;                                   // working variable

    short       nRegs;
    short       reqRegs[MAX_TRANSACTION_HRS];
    short       stAddr;
    short       turnOffTime;
    short       turnOnTime;
    short       val;

    cout << "LvLightController/processWrite> processing write request" << endl;

    memset (hrChanged, 0, sizeof(hrChanged));
    memset (reqRegs,   0, sizeof(reqRegs));
    stAddr = req->getStAddr();
    nRegs  = req->getNRegs();

    cout << "\t\t starting addr = " << stAddr << ";  # regs = " << nRegs << endl;
    
    for (int i=stAddr; i<(stAddr+nRegs); i++) {
        hrChanged[i] = TRUE;
        reqRegs[i]   = req->getReg(i-stAddr);
        cout << "\t\t reg ["<< i << "]changed, value=" << reqRegs[i]<< endl;

    }

    // Pre-load as much of the response buffer as we can
    val = htons (req->getTransID());
    memcpy ( (void *) &(resp[0]), (const void *) &val, sizeof (val) );
    
    val = htons (req->getProtoID());
    memcpy ( (void *) &(resp[2]), (const void *) &val, sizeof (val) );

    val = 3;                                                            // assume error => # of bytes to follow = 3
    val = htons (val);
    memcpy ( (void *) &(resp[4]), (const void *) &val, sizeof (val) );
    
    resp[6] = req->getUnitID();

    // Register 0 is controller mode
    if (hrChanged[0]) {
        cout << "\t\t hr[0] changed; hr[0] = " << reqRegs[0] << " hr[1] = " << reqRegs[1] << endl;

        switch (reqRegs[0]) {
            // Set controller mode to MANUAL and load the MANUAL COMMAND if it is sent too
            case CONTROLLER_MODE_MANUAL:
                if (nRegs == 1){
                    mb->setHR (MB_CONTROLLER_MODE, reqRegs[0]);
                    break;

                } else if (nRegs == 2) {
                    if (reqRegs[1] < 1 || reqRegs[1] > 3) {
                        resp[7] = req->getFC() | 0x80;
                        resp[8] = MB_ILLEGAL_DATA_VALUE;
                        return 9;
                    } else {
                
                        switch (reqRegs[1]) {
                            case MANUAL_COMMAND_OFF:
                                rc->setRelayOff();
                                mb->setHR (MB_CURRENT_STATE, (short) CURRENT_STATE_OFF);
                                break;
                            case MANUAL_COMMAND_ON:
                                rc->setRelayOn();
                                mb->setHR (MB_CURRENT_STATE, (short) CURRENT_STATE_ON);
                                break;
                            case MANUAL_COMMAND_BLINK:
                                // Blink is implemented in getLightChangeAction method
                                break;
                            default:
                                resp[7] = req->getFC() | 0x80;
                                resp[8] = MB_ILLEGAL_DATA_VALUE;
                                return 9;
                        }

                        // If we got this far, we processed a valid command => set the appropriate registers.
                        mb->setHR (MB_CONTROLLER_MODE, reqRegs[0]);
                        mb->setHR (MB_MANUAL_COMMAND,  reqRegs[1]);
                    }

                } else {
                    resp[7] = req->getFC() | 0x80;
                    resp[8] = MB_ILLEGAL_DATA_VALUE;
                    return 9;
                }
                break;

            // The following modes are valid so set the appropriate registers
            case CONTROLLER_MODE_AUTO_ABS_TIME:
                mb->setHR (MB_CONTROLLER_MODE, reqRegs[0]);
                break;

            case CONTROLLER_MODE_AUTO_DUSK_TO_DAWN:
                turnOnTime = 1440 + (short) dd->getDuskTime();
                mb->setHR (MB_TURN_ON_TIME, turnOnTime);

                turnOffTime = dd->getDawnTime();
                mb->setHR (MB_TURN_OFF_TIME, turnOffTime);
                
                mb->setHR (MB_CONTROLLER_MODE, reqRegs[0]);

                cout << "ProcessWrite>> setting dawn/dusk mode, on=" << (short) dd->getDuskTime()<< " off="<< turnOffTime<<endl;
                break;

            case CONTROLLER_MODE_AUTO_DUSK_TO_OFFSET:
                turnOnTime = 1440 + (short) dd->getDuskTime();
                mb->setHR (MB_TURN_ON_TIME, turnOnTime);

                mb->setHR (MB_CONTROLLER_MODE, reqRegs[0]);

                
                cout << "ProcessWrite>> setting dusk/delay mode, on=" << turnOnTime << " off="<< mb->getHR (MB_TURN_OFF_TIME) << endl;

                break;

            default:
                resp[7] = req->getFC() | 0x80;
                resp[8] = MB_INVALID_FUNCTION;
                return 9;
        }
    }

    // Register 2: On time
    if (hrChanged[2]) {
        if (reqRegs[2] >= 0 && reqRegs[2]<1441) {
            mb->setHR (MB_TURN_ON_TIME, reqRegs[2]);
        } else {
            resp[7] = req->getFC() | 0x80;
            resp[8] = MB_ILLEGAL_DATA_VALUE;
            return 9;
        }
    }


    // Register 3: Off time
    if (hrChanged[3]) {
        if (reqRegs[3] >= 0 && reqRegs[3]<1441) {
            mb->setHR (MB_TURN_OFF_TIME, reqRegs[3]);
        } else {
            resp[7] = req->getFC() | 0x80;
            resp[8] = MB_ILLEGAL_DATA_VALUE;
            return 9;
        }
    }

    // Register 4: Dusk-off time (# minutes after dusk to turn off)
    if (hrChanged[4]) {
        if (reqRegs[4] >= 0 && reqRegs[4]<1441) {
            mb->setHR (MB_DUSK_OFF_DELAY, reqRegs[4]);
        } else {
            resp[7] = req->getFC() | 0x80;
            resp[8] = MB_ILLEGAL_DATA_VALUE;
            return 9;
        }
    }

                                    
    // Register 5: Controller Longitude
    if (hrChanged[5]) {
        mb->setHR (MB_CONTROLLER_LONGITUDE,   reqRegs[5]);
        dd->setCalcReqd();

    } else {
        resp[7] = req->getFC() | 0x80;
        resp[8] = MB_ILLEGAL_DATA_VALUE;
        return 9;
    }

                            
    // Register 7: Controller Latitude
    if (hrChanged[7]) {
        mb->setHR (MB_CONTROLLER_LATITUDE,   reqRegs[7]);
        dd->setCalcReqd();

    } else {
        resp[7] = req->getFC() | 0x80;
        resp[8] = MB_ILLEGAL_DATA_VALUE;
        return 9;
    }
 
    // Register 9, 10: Current time
    if (hrChanged[9]) {
        if (hrChanged[10]) {
        
            mb->setHR (MB_CURRENT_TIME,   reqRegs[10]);
            mb->setHR (MB_CURRENT_TIME+1, reqRegs[9]);

        } else {
            resp[7] = req->getFC() | 0x80;
            resp[8] = MB_ILLEGAL_DATA_VALUE;
            return 9;
        }
    }

    // Register 11: Timezone
    if (hrChanged[11]) {
        if (reqRegs[11] > (-13) && reqRegs[11]<13) {
            mb->setHR (MB_TIMEZONE, reqRegs[11]);
        } else {
            resp[7] = req->getFC() | 0x80;
            resp[8] = MB_ILLEGAL_DATA_VALUE;
            return 9;
        }
    }

    // Register 12..15: IP Address
    if (hrChanged[12]) {
        if (hrChanged[13] && hrChanged[14] && hrChanged[15] ) {
        
            mb->setHR (MB_IP_OCTET_1,   reqRegs[12]);
            mb->setHR (MB_IP_OCTET_2,   reqRegs[13]);
            mb->setHR (MB_IP_OCTET_3,   reqRegs[14]);
            mb->setHR (MB_IP_OCTET_4,   reqRegs[15]);

            // TODO set IP address

        } else {
            resp[7] = req->getFC() | 0x80;
            resp[8] = MB_ILLEGAL_DATA_VALUE;
            return 9;
        }
    }

    //  Normal successful response message
    val = 6;
    val = htons (val);
    memcpy ( (void *) &(resp[4]), (const void *) &val, sizeof (val) );

    resp[7] = req->getFC();

    val = htons (req->getStAddr());
    memcpy ( (void *) &(resp[8]), (const void *) &val, sizeof (val) );

    val = htons (req->getNRegs());
    memcpy ( (void *) &(resp[10]), (const void *) &val, sizeof (val) );

    return 11;
}

//==================================================================
//  Check if the lights should be turned on or off
//==================================================================
int getLightChangeAction (int minutesSinceMidnight, ModbusRegisters *mb, DawnDusk *dd) {


    int         requiredAction; 

    short       currentState;
    short       desiredState;
    short       turnOnTime, turnOffTime;


    // Assume that no action is necessary 
    requiredAction = NO_ACTION_REQUIRED;
    desiredState   = currentState = mb->getHR(MB_CURRENT_STATE);

    cout << "./getLightChangeAction> current state = " << currentState << endl;
    cout << "./getLightChangeAction> controller mode = " << mb->getHR(MB_CONTROLLER_MODE) << endl;
    cout << "./getLightChangeAction>  manual command = " << mb->getHR(MB_MANUAL_COMMAND) << endl;

    // Check for a change condition based on the controller's current mode
    switch ( mb->getHR(MB_CONTROLLER_MODE) ) {

        case CONTROLLER_MODE_MANUAL:

            if (MANUAL_COMMAND_BLINK == mb->getHR(MB_MANUAL_COMMAND) ) {   

                if (CURRENT_STATE_ON == currentState) {
                    desiredState = CURRENT_STATE_OFF;

                } else {

                    desiredState = CURRENT_STATE_ON;
                }
            }
            break;

        case CONTROLLER_MODE_AUTO_ABS_TIME:

            if (minutesSinceMidnight >= mb->getHR(MB_TURN_ON_TIME)  && 
                minutesSinceMidnight < mb->getHR(MB_TURN_OFF_TIME) ) {
                
                desiredState = CURRENT_STATE_ON;

            } else {

                desiredState = CURRENT_STATE_OFF;
            }
                
            break;

        case CONTROLLER_MODE_AUTO_DUSK_TO_DAWN:

            turnOnTime = 1440 + (short) dd->getDuskTime();
            mb->setHR (MB_TURN_ON_TIME, turnOnTime);

            turnOffTime = dd->getDawnTime();
            mb->setHR (MB_TURN_OFF_TIME, turnOffTime);

            if (minutesSinceMidnight > turnOffTime ||
                minutesSinceMidnight < turnOnTime) {

                desiredState = CURRENT_STATE_OFF;

            } else {

                desiredState = CURRENT_STATE_ON;
            }
            break;

        case CONTROLLER_MODE_AUTO_DUSK_TO_OFFSET:

            if (minutesSinceMidnight > dd->getDuskTime() &&
                minutesSinceMidnight < (dd->getDuskTime() + mb->getHR(MB_DUSK_OFF_DELAY) ) ) {

                desiredState = CURRENT_STATE_ON;

            } else {

                desiredState = CURRENT_STATE_OFF;
            }
            break;

        default:
            break;
    }

    if (CURRENT_STATE_ON  == desiredState  &&
        CURRENT_STATE_OFF == currentState) { 

            requiredAction = TURN_LIGHTS_ON;
        
    } else if (CURRENT_STATE_OFF == desiredState &&                     // lights should be off
               CURRENT_STATE_ON  == currentState) { 

            requiredAction = TURN_LIGHTS_OFF;
    }

    return requiredAction; 
}


//==================================================================
//  Main line
//==================================================================
int main ()
{
    int         gmtoff;
    int         lvState;
    int         minutesSinceMidnight;
    int         oldDay;
    int         processingTransaction;
    int         respSz;
    int         requiredAction;
    int         status;
    int         x;

    char        resp[255];

    time_t      timeNow;                                                // seconds since the epoch
    struct tm   timeInfo;

    // Create the necessary objects
    LvLightController lvCtlr;
    RelayControl      rc;
    NwComms           nw;
    ModbusRegisters   mb;
    DawnDusk          dd;

    oldDay = 0;

    // Controller (like rust) never sleeps
    while (TRUE) {

    
        // Get current time
        time (&timeNow);
        localtime_r (&timeNow, &timeInfo);
//      timeInfo             = localtime_r (&timeNow);
        gmtoff = timeInfo.tm_gmtoff;
        minutesSinceMidnight = (timeInfo.tm_hour * 60) + timeInfo.tm_min;
            
   
        if (timeInfo.tm_mday != oldDay) {
            cout << "LVLightController> calculating dawn & dusk, gmtoff = " << gmtoff << endl;
            status = dd.calculateDawnAndDuskTimes (&mb, &timeInfo, gmtoff);
            dd.clrCalcReqd();
        }

        oldDay = timeInfo.tm_mday;
        
        processingTransaction = TRUE;

        while (processingTransaction) {
            cout << "===========================================" << endl;

            // Instantiate transaction objects - they will be deleted when the transaction is complete
            ModbusTransaction req;

            status = nw.getRequest (&req);
            cout << "LVLightController> request =" << req.getFC() << endl;

            if (NW_NETWORK_ERROR == status) {
                cout << "LVLightController> N/W error" << endl;

            } else if (NW_CONNECTION_ACCEPTED == status) {              
                continue;                                               // expected - call getRequest again to retrieve data
            
            } else if (NW_PEER_RESET_CONNECTION == status) {
                nw.closeConnection();
            
            
            } else if (NW_VALID_REQUEST == status) {

                switch ( (int) req.getFC() ) {

                    case FC_READ_MULTIPLE_REGISTERS:
                        respSz = processRead (&req, resp, &mb);
                        nw.sendResponse (resp, respSz);
                        break;

                    case FC_WRITE_MULTIPLE_REGISTERS:
                        respSz = processWrite (&req, resp, &mb, &rc, &dd);
                        nw.sendResponse (resp, respSz);
                        mb.saveHR();
                        break;

                    default:
                        break;
                }
            }

            processingTransaction = FALSE;                              // wait for next xaction
        }                                                               // while processing xaction

        // Re-calculate dawn & dusk times if necessary
        if (dd.getCalcReqd() == TRUE) {
            status = dd.calculateDawnAndDuskTimes (&mb, &timeInfo, gmtoff);
            dd.clrCalcReqd();
        }

        // Normal, non-transaction processing (e.g., is it time to turn on or off?)
        requiredAction = getLightChangeAction (minutesSinceMidnight, &mb, &dd);

        cout << "lightcontroller> requiredAction = " << requiredAction << endl;

        if (requiredAction == TURN_LIGHTS_ON) {

            rc.setRelayOn();
            mb.setHR (MB_CURRENT_STATE, (short) CURRENT_STATE_ON);
            mb.saveHR();

        } else if (requiredAction == TURN_LIGHTS_OFF) {
        
            rc.setRelayOff();
            mb.setHR (MB_CURRENT_STATE, (short) CURRENT_STATE_OFF);
            mb.saveHR();
        }
    }                                                                   // while true
    
    cout << "LvLightController exiting - press any key to exit" << endl;
    x = fgetc(stdin);
    return 0;
}
