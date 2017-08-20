#include "LV_Lights.h"
#include "modbusTransaction.h"
#include "modbusRegisters.h"
#include <string.h>
#include <arpa/inet.h>

// Constructor
ModbusTransaction::ModbusTransaction () {
    cout << "ModbusTransaction> initializing..." << endl;
    fc = 0;
}

ModbusTransaction::~ModbusTransaction() {
    cout << "ModbusTransaction> Executing destructor" << endl;
}

#if 0
short getFC() {
    return fc;
}

short getStAddr() {
    return stAddr;
}


short getNRegs() {
    return nRegs;
}

short getReg (int regNo) {
    return regs[regNo];
}
#endif

int ModbusTransaction::parseStream(int rcvBytes, char *stream) {

    static int      savedState = {0};
    int             state;
    
    state    = savedState;

    cout << "ModbusTransaction> parsing..." << endl;

    // State machine to parse stream
    for (int i=state; i<rcvBytes; i++) {

//      if ( (rcvBytes-1) == i) lastByte = TRUE;

        cout << endl << "State " << state << " byte[" << i << "] " << (int) stream[i] ;
        switch (state) {

            // State 0 - get transaction ID - byte 1
            case 0:
                transID = stream[i]<<8;
                state++;
                break;


            // State 1 - get transaction ID - byte 2
            case 1:
                transID |= stream[i];
                cout << "\ttransID = " << transID;
                state++;
                break;

            // State 2 - check protocol ID (Modbus = 0) - byte 1
            case 2:
                protoID = stream[i]<<8;
                state++;
                break;

            // State 3 - check protocol ID (Modbus = 0) - byte 2
            case 3:
                protoID |= stream[i];
                cout << "\t\tprotocol = " << protoID;
                if (0 == protoID) {
                    state++;
                } else {
                    savedState = 0;
                    return MB_INVALID_MBAP;
                }
                break;


            // State 4 - retrieve the request length (bytes) - byte 1
            case 4:
                length = stream[i]<<8;
                state++;
                break;

            // State 5 - retrieve the request length (bytes) - byte 2
            case 5:
                length |= stream[i];
                state++;

                cout << "\t\t rcvBytes=" << rcvBytes << " length=" << length ;

                if (rcvBytes != length+6) {
                    savedState = state;
                    return MB_INCOMPLETE_REQUEST;
                }
                break;

            // State 6 - retrieve the Unit ID (not typically used for TCP implementations)
            case 6:
                unitID = stream[i];

                cout << "\t\t unitID=" << (int) unitID ;

                state++;
                break;

            // State 7 - retrieve the Function Code
            case 7:
                fc = stream[i];

                cout << "\t fc=" << (int) fc ;

                if (FC_READ_MULTIPLE_REGISTERS != fc && FC_WRITE_MULTIPLE_REGISTERS != fc) {
                    savedState = 0;
                    return MB_INVALID_FUNCTION;
                } else {
                    state++;
                    break;
                }


            // State 8 - get the starting register number - byte 1
            case 8:
                stAddr = stream[i]<<8;
                state++;
                break;

            // State 9 - get the starting register number - byte 2
            case 9:
                stAddr |= stream[i];

                cout << "\t\t stAddr=" << stAddr ;

                if (0 > stAddr || MB_MAX_HRS < stAddr) {
                    savedState = 0;
                    return MB_ILLEGAL_ADDRESS;
                } else {
                    state++;
                }
                break;


            // State 10 - get the number of registers - byte 1
            case 10:
                nRegs = stream[i]<<8;
                state++;
                break;

            // State 11 - get the number of registers - byte 2
            case 11:
                nRegs |= stream[i];

                cout << "\t\t nRegs=" << nRegs ;

                if (FC_READ_MULTIPLE_REGISTERS == fc) {
                    savedState = 0;
                    return MB_VALID_REQUEST;
                }
                state++;
                break;

            // State 12 - get the number of bytes
            case 12:
                nBytes = stream[i];

                cout << "\t\t nBytes=" << (int) nBytes;

                state++;
                break;

            // State 13 - load the registers
            case 13:
                memcpy ( (void *) &(regs[0]), (const void *) &stream[i], nRegs*2);
                cout << "\t\t regs (from stream[" << i <<"]:" << endl << "\t\t" ;
                
                for (int r=0; r<nRegs; r++) {
                    regs[r] = ntohs (regs[r]);
                    cout << "regs[" << r << "]=" << regs[r] << " ";
                }
                savedState = 0;
                return MB_VALID_REQUEST;
        }                                                   // switch
    }                                                       // for i..

    // Return error if we get this far
    savedState = state;
    return MB_INCOMPLETE_REQUEST;                           // need more bytess
}


