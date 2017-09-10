#ifndef DAWN_DUSK_H
#define DAWN_DUSK_H

class DawnDusk {

// Definitions


public:
    DawnDusk () ;
    ~DawnDusk () ;

    int getDuskTime (){
        return duskMinutesAfterMidnight;
    };

    int getDawnTime (){
        return dawnMinutesAfterMidnight;
    };

    int setCalcReqd (){
        calcReqd = TRUE;
        return 0;
    };

    int clrCalcReqd (){
        calcReqd = FALSE;
        return 0;
    };

    int getCalcReqd (){
       return  calcReqd;
    };

    int calculateDawnAndDuskTimes (ModbusRegisters *mb, struct tm *timeInfo, int gmtoff);


private:
    float calculateSunEventTime ( int yday, float lngHour, float latitude, float utcOffset, int dawnOrDusk);

    int  duskMinutesAfterMidnight;
    int  dawnMinutesAfterMidnight;

    int  calcReqd;
};
#endif
