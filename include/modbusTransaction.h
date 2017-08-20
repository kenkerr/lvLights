#ifndef MB_TRANSACTION_H
#define MB_TRANSACTION_H

class ModbusTransaction {

// Return Definitions
#define MB_VALID_REQUEST            0x00
#define MB_INVALID_FUNCTION         0x01
#define MB_ILLEGAL_ADDRESS          0x02
#define MB_ILLEGAL_DATA_VALUE       0x03

#define MB_INVALID_MBAP             0x20
#define MB_INVALID_REQUEST          0x21
#define MB_INCOMPLETE_REQUEST       0x22

#define MB_METHOD_SUCCESS           0x00

// Modbus Function Codes
#define FC_READ_MULTIPLE_REGISTERS  0x03
#define FC_WRITE_MULTIPLE_REGISTERS 0x10

// Misc constants
#define MAX_TRANSACTION_HRS         32

public:
    ModbusTransaction ();
    ~ModbusTransaction();

    short getFC(){
        return fc;
    };
    int setFC(short newFC){
        fc = newFC;
        return MB_METHOD_SUCCESS;
    }

    short getTransID () {
        return transID;
    };
    short getProtoID () {
        return protoID;
    };
    short getlength();
    short setlength();
    char  getUnitID() {
        return unitID;
    };

    short setUnitID();
    short getStAddr(){
        return stAddr;
    };
    int setStAddr(short newStAddr) {
        stAddr = newStAddr;
        return MB_METHOD_SUCCESS;
    }
    short getNRegs(){
        return nRegs;
    };
    int setNRegs(short newNRegs) {
        nRegs = newNRegs;
        return MB_METHOD_SUCCESS;
    }
    short getNBytes();
    short setNBytes();
    short getReg(int regNo){
        return regs[regNo];
    };
    int setReg(int regNo, short regVal) {
        regs[regNo] = regVal;
    };


    int   parseStream (int nBytes, char *stream);


private:
// MBAP (ModBus Application Protocol - TCP header)
    short transID;
    short protoID;
    short length;
    char  unitID;

// PDU (Protocol Data Unit)
    char  fc;
    short stAddr;
    short nRegs;
    char  nBytes;
    short regs[MAX_TRANSACTION_HRS];

// Response
    char excCode;

};
#endif
