// Calculations taken from the Sunrise/Sunset Algorithm, published
// by Nautical Almanac Office.  See willams.best.vwh.net/sunrise_sunset_algorithm.htm

#include "LV_Lights.h"
#include "modbusRegisters.h"
#include "dawnDusk.h"

#define  DAWN       1
#define  DUSK       2

#define rad2Deg(rad) (rad * 180. / 3.1414927)
#define deg2Rad(deg) (deg * 3.1515927 / 180.)

// Constructor
DawnDusk::DawnDusk () {

    cout << "DawnDusk> initializing..." << endl;

};

DawnDusk::~DawnDusk() {
    cout << "DawnDusk> Executing destructor" << endl;
};

float adjust (float inputValue, float upperLimit) {

    if (inputValue < 0.) {
        return (inputValue+upperLimit);
    } else if (inputValue > upperLimit) {
        return (inputValue-upperLimit);
    }
    return inputValue;
}

float DawnDusk::calculateSunEventTime (int yday, float lngHour, float latitude, float utcOffset, int dawnOrDusk) {

    float   cosDec;
    float   cosH;
    float   H;
    float   L;
    float   localT;
    float   Lquadrant;
    float   M;
    float   RA;
    float   RAquadrant;
    float   sinDec;
    float   t;
    float   T;                                                          // local mean time of event
    float   UT;                                                         // GMT mean time of event
    float   zenith = {90.833};                                          // 90 deg 50'

    // 2b. Calc an approx time
    if (DAWN == dawnOrDusk) {
        t = yday + ((6 - lngHour) / 24);
    } else {
        t = yday + ((18 - lngHour) / 24);
    }

    // 3. Calculate the sun's mean anomaly
    M = (0.9856 * t) - 3.289;

    // 4. Calculate the Sun's true longitude
    L = M + (1.916 * sin( deg2Rad(M) )) + (0.020 * sin(2 * deg2Rad(M) )) + 282.634;
    L = adjust (L, 360.);

    // 5a. calculate the Sun's right ascension
    RA = rad2Deg ( atan(0.91764 * tan(deg2Rad(L) )) );
        cout << "RA" << RA << endl;
    RA = adjust (RA, 360.);
        cout << "RA" << RA << endl;

    // 5b. right ascension value needs to be in the same quadrant as L
    Lquadrant  = (floor( L/90)) * 90;
    RAquadrant = (floor(RA/90)) * 90;
    RA = RA + (Lquadrant - RAquadrant);
        cout << "RA" << RA << endl;

    // 5c. right ascension value needs to be converted into hours
    RA = RA / 15;

    // 6. calculate the Sun's declination
    sinDec = 0.39782 * sin(deg2Rad(L) );
    cosDec = cos(asin(sinDec));
                                                        
    // 7a. calculate the Sun's local hour angle
    cosH = (cos(deg2Rad(zenith) ) - (sinDec * sin(deg2Rad(latitude)) )) / (cosDec * cos(deg2Rad(latitude)) );
                                                                    
#if 0 //- don't worry about these locations right now
    if (cosH >  1) 
//      the sun never rises on this location (on the specified date)
    if (cosH < -1)
//      the sun never sets on this location (on the specified date)
#endif

    // 7b. finish calculating H and convert into hours
    if (DAWN == dawnOrDusk) {
        H = 360 - rad2Deg( acos(cosH) );
    } else {
        H = rad2Deg( acos(cosH) );
    }

    H = H / 15;

    // 8. calculate local mean time of rising/setting
    T = H + RA - (0.06571 * t) - 6.622;

    // 9. adjust back to UTC
    UT = T - lngHour;
    UT = adjust (UT, 24.);

    // 10. convert UT value to local time zone of latitude/longitude
    localT = UT + utcOffset;

    cout << "t, M, L, RA, Lquadrant, RAquadrant, sinDec, cosDec, cosH, H, T, UT " 
<< t << " " << M << " " << L << " " << RA << " " << Lquadrant << " " << RAquadrant << " " 
<< sinDec << " " << cosDec << " " << cosH << " " << H << " " << T << " " << UT << endl;
    return localT;
}


int DawnDusk::calculateDawnAndDuskTimes (ModbusRegisters *mb, struct tm *timeInfo, int gmtoff) {
    
    float   dawnTime;
    float   duskTime;
    float   lngHour;
    float   utcOffset;                                                  // in hours

    int     yday;
    int     longitude;
    int     latitude;
    int     n, n1, n2, n3;

    longitude = mb->getHR(MB_CONTROLLER_LONGITUDE);
    latitude  = mb->getHR(MB_CONTROLLER_LATITUDE);

    longitude = -80;
    latitude  = 37;

#if 0
    // 1. Calculate the day of the year
    n1 = (int) (275 * timeInfo->tm_mon / 9);
    n2 = (int) ((timeInfo->tm_mon + 9) / 12);
    n3 = (1 + (int) ((timeInfo->tm_year - 4 * (int) (timeInfo->tm_year/4) + 2) / 3)):
    n  = n1 - (n2 * n3) + timeInfo->tm_day - 30;
#endif

    // Get day of year
    yday      = timeInfo->tm_yday;
    utcOffset = gmtoff / 3600.;                               // GMT offset in hours

    // 2a. Convert the longitude to hour value
    lngHour = longitude / 15.;

    cout << "yday, utcOffset, lngHour " << yday << " " << utcOffset << " " << lngHour <<endl;

    duskTime = calculateSunEventTime (yday, lngHour, (float) latitude, utcOffset, DUSK);
    dawnTime = calculateSunEventTime (yday, lngHour, (float) latitude, utcOffset, DAWN);

    cout << "DawnDusk> dawn, dusk = " << dawnTime << " " << duskTime << endl;
    duskMinutesAfterMidnight  = (int) (duskTime * 60);
    dawnMinutesAfterMidnight  = (int) (dawnTime * 60);

    cout << "DawnDusk> dawn, dusk (mins) = " << dawnMinutesAfterMidnight << " " << duskMinutesAfterMidnight << endl;

    return 0;
}
