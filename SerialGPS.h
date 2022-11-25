#include "mbed.h"

#ifndef MBED_GPS_H
#define MBED_GPS_H

void gps_task(void);
/**  A SerialGPS interface for reading from a serial GPS module */
class SerialGPS {
public:

    /** Create the SerialGPS interface, connected to the specified serial port and speed.
     *  for example, GlobalSat EM406-A (e.g. on SparkFun GPS Shield) is 4800 Baud,
     *  Adafruit Ultimate GPSv3 (connected to serial) is 9600 Baud
     */    
    SerialGPS(PinName tx, PinName rx);
    
    /** Sample the incoming GPS data, returning whether there is a lock
     * 
     * @return 1 if there was a lock when the sample was taken (and therefore .longitude and .latitude are valid), else 0
     */
    int sample();

    void PrintTime();
    void PrintLocation();
    
    /** The longitude (call sample() to set) */
    float longitude;

    /** The latitude (call sample() to set) */
    float latitude;
    
    /** The time (call sample() to set) */
    float time;
    
    /** Number of satellites received (call sample() to set) */
    int sats;
    
    /** Horizontal dilusion of precision (call sample() to set) */
    float hdop;
    
    /** The altitude (call sample() to set)
     *  Note that the accurate altitude is corrected by the geoid
     *  See http://homepages.slingshot.co.nz/~geoff36/datum.htm
     */
    float alt;
    
    /** The geoid (call sample() to set) */
    float  geoid;
    
    /** The NMEA sentence */
    char msg[256];
    
    int h,m,s;


    
private:
    float trunc(float v);
    void getline();
    
    Serial _gps;
};

extern SerialGPS myGPS;
#endif