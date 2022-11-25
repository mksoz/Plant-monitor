 
#include "SerialGPS.h"
#include "math.h"



SerialGPS::SerialGPS(PinName tx, PinName rx) : _gps(tx, rx) {
    longitude = 0.0;
    latitude = 0.0;
    time = 0.0;
    h = m = s = 0;        
}

int SerialGPS::sample() {
    char ns, ew, unit;
    int lock;

    while(1) {        
        getline();

        // Check if it is a GPGGA msg (matches both locked and non-locked msg)
        if(sscanf(msg, "GPGGA,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c,%f", &time, &latitude, &ns, &longitude, &ew, &lock, &sats, &hdop, &alt, &unit, &geoid) >= 1) { 
           //GPS sentence Message ID: GPGGA -> UTC time, Latitude, N/S Indicator (North - South), Longitude, E/W Indicator (East - West), Position Fix Indicator, Satellites used(rannge is 0 to 12), HDOP(Horizontal Dilution of Precision), MSL Altitude, Units, Geoid Separation
            //Position Fix Indicator value:
                //                             0 -> Fix not available or invalid
                //                             1 -> GPS SPS Mode. fix valid
                //                             2 -> Differential GPS, SPS Mode, fix valid
                //                             3-5 ->  Not supported
                //                             6 -> Dead Reckoning Mode, fix valid
             if(!lock) {
                time = 0.0;
                longitude = 0.0;
                latitude = 0.0;
                sats = 0;
                hdop = 0.0;
                alt = 0.0;
                geoid = 0.0;
                h = m = s = 0;        
                return 0;
            } else {
                //GPGGA format according http://aprs.gids.nl/nmea/#gga
                // time (float), lat (f), (N/S) (c), long (f), (E/W) (c), fix (d), sats (d),
                // hdop (float), altitude (float), M, geoid (float), M, , ,  
                //GPGGA,092010.000,5210.9546,N,00008.8913,E,1,07,1.3,9.7,M,47.0,M,,0000*5D
                h = time/1000;
                m = (time - h*1000)/100;
                s = (time - h*1000 -m*100);



                if(ns == 'S') {    latitude  *= -1.0; }//if South latitude = negative
                if(ew == 'W') {    longitude *= -1.0; }//if West longitud = negative
                float degrees = trunc(latitude / 100.0f);//lat_rx ddmm.mmmm (degrees dd)
                float minutes = latitude - (degrees * 100.0f);//(minutes mm.mmmm)
                latitude = degrees + minutes / 60.0f;    
                degrees = trunc(longitude / 100.0f * 0.01f);//longitud ddmm.mmmm (degrees dd)
                minutes = longitude - (degrees * 100.0f);//(minutes mm.mmmm)
                longitude = degrees + minutes / 60.0f;
                return 1;// NMEA GGA -> Time, position and fix type data.
            }
        }
    }
}

float SerialGPS::trunc(float v) {
    if(v < 0.0) {
        v*= -1.0;
        v = floor(v);
        v*=-1.0;
    } else {
        v = floor(v);
    }
    return v;
}

void SerialGPS::getline() {
    while(_gps.getc() != '$');   // wait for the start of a line
    for(int i=0; i<256; i++) {
        msg[i] = _gps.getc();
        if(msg[i] == '\r') {
            msg[i] = 0;
            return;
        }
    }
    error("Overflowed message limit");
}

void SerialGPS::PrintTime(){
    printf("GPS time: %02d:%02d:%02d\n", h,m,s);
}
void SerialGPS::PrintLocation(){
    printf("GPS Location:\n");
    printf(" -Latitude (UTC): %.2f\n",latitude);
    printf(" -Longitude (UTC): %.2f\n",longitude);
    printf(" -Altitude: %.1f m\n", alt);
}

void gps_task(){
   myGPS.sample(); 
}