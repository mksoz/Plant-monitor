/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "I2C.h"
#include "mbed.h"
#include "InterruptIn.h"
#include "Ticker.h"
#include "PinNames.h"
#include <cstdio>
#include "lib.h"
#include "SerialGPS.h"

//-------Adresses-----------
const int addrColor  = 0x29 << 1;  //Color slave
const int addrTemHum = 0x40 << 1;  //TempHum slave
const int addrAcc    = 0x1C << 1;  //Accelerometer slave
//--------Pines------------ 
I2C accel(PB_9, PB_8);           //Accelerometer
I2C i2cColor(PB_9,PB_8);         //Color sensor
DigitalOut green(PB_7);          
I2C i2cTemp(PB_9,PB_8);          //Temp_Si7021s
DigitalOut led1(LED1);           //LED1
DigitalOut led2(LED2);           //LED2
AnalogIn soil(PA_0);             //Soil moisure
AnalogIn light_sensor(PA_4);     //Light Sensor
BusOut rgb(PH_0, PH_1, PB_13);   //RGB LED
SerialGPS myGPS(PA_9,PA_10);    //GPS
//User button
InterruptIn button(PB_2);

//-------Variables ---------
uint16_t temp_code, hum_code;
int16_t acc_1, acc_2, acc_3;
char buf[8], cmd_tx[1], cmd_rx[2];
char cmd_color[1];
char x_data[2], y_data[2], z_data[2];
int state = INIT, cont;
bool stateFlag, measureFlag = false;
Ticker tick;
float lightValue, soilValue, acc_1_float, acc_2_float, acc_3_float, tempValue, humValue;
char cmdColorInit[2];
uint16_t r, g, b, c;
uint16_t reg_color[3];
char msg_print[MSG_TO_RINT_LEN] = {0};
char ctrl_reg1[2];
char x_regaddr[1] = {REG_OUT_X_MSB}, y_regaddr[1] = {REG_OUT_Y_MSB}, z_regaddr[1] = {REG_OUT_Z_MSB};  
float reg_temp[3], reg_hum[3], reg_light[3], reg_soil[3], reg_acc[6];



void func_flag(){
    stateFlag = !stateFlag;
}

void func_measure(){
    measureFlag = !measureFlag;
}

void statistics_measures(){

    //-------Temperature----------
    if (tempValue > reg_temp[0]){
        reg_temp[0]=tempValue;
    }
    if (tempValue < reg_temp[1]) {
        reg_temp[1]=tempValue;
    }
    reg_temp[2] += tempValue;
    //----------Humidity----------
    if (humValue > reg_hum[0]){
        reg_hum[0]=humValue;
    }
    if (humValue < reg_hum[1]) {
        reg_hum[1]=humValue;
    }
    reg_hum[2] += humValue;
    //------------Light------------
    if (lightValue > reg_light[0]){
        reg_light[0]=lightValue;
    }
    if (lightValue < reg_light[1]) {
        reg_light[1]=lightValue;
    }
    reg_light[2] += lightValue;
    //-------------Soil-------------
    if (soilValue > reg_soil[0]){
        reg_soil[0]=soilValue;
    }
    if (soilValue < reg_soil[1]) {
        reg_soil[1]=soilValue;
    }
    reg_soil[2] += soilValue;
    //----------Accelerometer--------
    if (acc_1_float > reg_acc[0]){
        reg_acc[0]=acc_1_float;
    }
    if (acc_1_float< reg_acc[1]) {
        reg_acc[1]=acc_1_float;
    }
     if (acc_2_float > reg_acc[2]){
        reg_acc[2]=acc_2_float;
    }
    if (acc_2_float< reg_acc[3]) {
        reg_acc[3]=acc_2_float;
    }
     if (acc_3_float > reg_acc[4]){
        reg_acc[4]=acc_3_float;
    }
    if (acc_3_float< reg_acc[5]) {
        reg_acc[5]=acc_3_float;
    }
    //-----------Color--------------
    reg_color[0] += r;
    reg_color[1] += g;
    reg_color[2] += b;
    //-----------counter------------
    cont += 1;
}


void measure_sensors(){
    
    if (measureFlag){
        //Analogs
        lightValue = light_sensor.read();
        soilValue  = soil.read();
        
        // temperature
        cmd_tx[0] = 0xE3;
        i2cTemp.write(addrTemHum, cmd_tx, 1, false);
        i2cTemp.read( addrTemHum, cmd_rx, 2, false);
        temp_code = (cmd_rx[0] << 8) | cmd_rx[1] ;
        tempValue = ((175.72*temp_code)/65536)-46.85;
        // humidity
        cmd_tx[0] = 0xE5;
        i2cTemp.write(addrTemHum, cmd_tx, 1, false);
        i2cTemp.read( addrTemHum, cmd_rx, 2, false);
        hum_code = (cmd_rx[0] << 8) | cmd_rx[1] ;
        humValue = ((125*hum_code)/65536)-6;
        
        // COLOR
        /*To ensure the data is read correctly,  
        a read word protocol bit set in the command register
        RGBC Channel Data Registers (0x14 − 0x1B)*/
        cmd_color[0] = {0x14 | 0x80}; 
        i2cColor.write(addrColor, cmd_color, 1);
        i2cColor.read( addrColor, buf,       8);
        //Clear, red, green, and blue data is stored as 16-bit values.
        //the upper eight bits are stored into a shadow register
        c=(buf[1]<<8 |buf[0]);
        r=(buf[3]<<8 |buf[2]);
        g=(buf[5]<<8 |buf[4]);
        b=(buf[7]<<8 |buf[6]);

        
        accel.write(addrAcc, x_regaddr, 1, true); //Repeated start, true - do not send stop at end
        accel.read(addrAcc,  x_data,    2);
        acc_1 = (x_data[0] << 6) | (x_data[1] >> 2);
        //        XXXXXXXX             X_DATA[0] MSB
        //   XXXXXXXX000000 <<6
        //                  XXXXXX00   X_DATA[1] LSB
        //                  00XXXXXX   X_DATA[1] >> 2
        if(acc_1 > DIV)
        {
            acc_1 -=16384;
        }
        /*BIT 14 represent the sign, however because of the read is in 16 bits the number is consider pos
        Check if the number is larger than the half the numbers we are able to represent in 2nd compl
        if it is the, the num is negative */
        acc_1_float = float (acc_1/4096.0) * GRAVITY;
        //converting 2g 2 compl hex value to decimal value in g
        
        accel.write(addrAcc, y_regaddr, 1, true); 
        accel.read( addrAcc, y_data,    2); 
        acc_2 = (y_data[0] << 6) | (y_data[1] >> 2);
            
        if(acc_2 > DIV)
        {
            acc_2 -=16384;
        }
        acc_2_float = float (acc_2/4096.0) * GRAVITY;

        accel.write(addrAcc, z_regaddr, 1, true); 
        accel.read( addrAcc, z_data,    2); 
        acc_3 = (z_data[0] << 6) | (z_data[1] >> 2);
            
        if(acc_3 > DIV)
        {
            acc_3 -=16384;
        }
	    acc_3_float = float (acc_3/4096.0) * GRAVITY;
    
    }
}

void init_reg(){
// Initialize register for max and min values
    measureFlag = true;
    measure_sensors();
    measureFlag  = false;
    reg_temp[0]  = reg_temp[1]  = tempValue;
    reg_hum[0]   = reg_hum[1]   = humValue;
    reg_light[0] = reg_light[1] = lightValue;
    reg_soil[0]  = reg_soil[1]  = soilValue;
    reg_acc[0]   = reg_acc[1]   = acc_1_float;
    reg_acc[2]   = reg_acc[3]   = acc_2_float;
    reg_acc[4]   = reg_acc[5]   = acc_3_float;
    reg_temp[2]  = reg_hum[2]   = reg_light[2] = reg_soil[2] = 0;
    reg_color[0] = reg_color[1] = reg_color[2]= 0;
    cont = 0;
}

void printMeasures (){   
printf("\nSOIL MOISTURE: %f %", soilValue*100);
printf("\nLIGHT: %f %", lightValue*100);
printf("\nTime: %f", myGPS.time);
printf("\nLocation. LAT: %f LONG: %f  ALT: %f", myGPS.latitude, myGPS.longitude, myGPS.alt);
printf("\nCOLOR SENSOR: Clear: %d, Red: %d, Green: %d, Blue %d",c,r,g,b);
printf("\nACCELEROMETER: X_axis: %f, Y_axis: %f, Z_axis: %f",acc_1_float, acc_2_float, acc_3_float);
printf("\nTEMP/HUM: Temperature: %f C Relative Humidity: %f %", tempValue, humValue);
printf("\n");
}

void printStatistic(){
    if(cont == HOUR){
        printf("\n---STATISTICS---");
        printf("\nMax Temperature: %f, Min Temperature: %f, Mean Temperature: %f", reg_temp[0], reg_temp[1], reg_temp[2]/cont);
        printf("\nMax Rel.Humidity: %f, Min Rel.Humidity: %f, Mean Rel.humidity: %f", reg_hum[0], reg_hum[1], reg_hum[2]/cont);
        printf("\nMax Light: %f %, Min Light: %f %, Mean Light: %f %", reg_light[0]*100, reg_light[1]*100, (reg_light[2]/cont)*100);
        printf("\nMax Soil: %f, Min Soil: %f, Mean Soil: %f", reg_soil[0]*100, reg_soil[1]*100, (reg_soil[2]/cont)*100);
        printf("\nMax X-Axe: %f, Min X-Axe: %f", reg_acc[0], reg_acc[1]);
        printf("\nMax Y-Axe: %f, Min Y-Axe: %f", reg_acc[2], reg_acc[3]);
        printf("\nMax Z-Axe: %f, Min Z-Axe: %f", reg_acc[4], reg_acc[5]);
        if (reg_color[0]>reg_color[1] && reg_color[0]>reg_color[2]) {
            printf("\nDominant color-> RED: %d\n",reg_color[0]/cont);
        }else {
        if(reg_color[1]>reg_color[0] && reg_color[1]>reg_color[2]) {
            printf("\nDominant color-> GREEN: %d\n",reg_color[1]/cont);
        }else {
            printf("\nDominant color-> BLUE: %d\n",reg_color[2]/cont);
        }
        }
        //Reset registers
        init_reg();
        //Reset counter
        cont = 0;
    }
}

//Init GPS task

void init_slaves(){
    
    // Color sensor writes
    /* Enable with PON
    If PON disabled, the device will return to the Sleep state to save power */
    cmdColorInit[0]= 0x00 | 0x80;
    cmdColorInit[1]=0x01;
    i2cColor.write(addrColor,cmdColorInit,2);
    
    /*
    Active AEN
    device will remain in the Idle state until the RGBC function is enabled (AEN)
    Upon completion and return to Idle, the device will automatically begin 
    a new Wait-RGBC cycle as long as PON and AEN remain enabled.*/
    cmdColorInit[1]=0x01|0x02;
    i2cColor.write(addrColor,cmdColorInit,2); //start condition is detected on the I2C bus
    
    // Accelerometer
    /* The most significant 8-bits of each axis are stored in OUT_X (Y,
    Z)_MSB, so applications needing only 8-bit results can use these 3 
    registers and ignore OUT_X,Y, Z_LSB. To do this, the
    F_READ bit in CTRL_REG1 must be set */
    ctrl_reg1[0] = REG_CTRL_REG;
    ctrl_reg1[1] = 0x01;
    accel.write(addrAcc, ctrl_reg1, 2, true);


}


void swapRGB(){
    //lightValue, soilValue, acc_1_float, acc_2_float, acc_3_float, tempValue, humValue
    if (state==2) {
    
        if (tempValue>MAX_TEMP || tempValue<MIN_TEMP) {
            rgb.write(1);
            printf("\ntemp");
        }else{
        if (humValue>MAX_HUM || humValue<MIN_HUM) {
            rgb.write(4);printf("\nhum");
        }
        else{
        if (soilValue>MAX_SOIL || soilValue<MIN_SOIL) {
            rgb.write(2);printf("\nsoil");
        }
        else{
        if (lightValue>MAX_LIGHT || lightValue<MIN_LIGHT) {
            rgb.write(3);printf("\nLight");
        }else{
        if (acc_1_float>MAX_X || acc_1_float<MIN_X) {
            rgb.write(5);printf("\nX");
        }else {
        if (acc_2_float>MAX_Y || acc_2_float<MIN_Y) {
            rgb.write(6);printf("\nY");
        }else {
        if (acc_3_float>MAX_Z || acc_3_float<MIN_Z) {
            rgb.write(7);printf("\nZ");
        }else {
            rgb.write(0);printf("\nNone");
        }}}}}}}

    }else {
    if(g>r && g>b){
        rgb.write(2);
    }
    else{
        if(r>g && r>b){
            rgb.write(1);
        }
        else{ 
            rgb.write(4);}
            }
}
    }
    

int main()
{
    
    init_slaves();    
    // Initialize state and button
    state = INIT;
    button.mode(PullUp);
    button.fall(func_flag); //button to change modes
    
    while (true) {
        switch (state) {
            case INIT:
                if (!stateFlag) {
                    tick.attach_us(func_measure, TWO_SEC);
                    led1 = 1;
                    led2 = 0;
                    state = TEST;
                    printf("\n\t--TEST MODE--");
            }else {
                tick.attach_us(func_measure, THIRTY_SEC);
                led1 = 0;
                led2 = 1;
                init_reg();
                rgb.write(0);
                state = NORMAL;
                printf("\n\t--NORMAL MODE--");
            }break;
        
            case TEST:
                if(stateFlag) {
                    state = INIT;
                  tick.detach();
                  break;
                }else {
                    if (measureFlag) {
                        measure_sensors();
                        measureFlag = false;
                        printMeasures();
                        swapRGB();
                        }
                    state = TEST;
                    break;
                }
                     
            case NORMAL:
                if(!stateFlag) {
                    state = INIT;
                    tick.detach();
                    break;
                    }else {
                        if (measureFlag) {
                            measure_sensors();
                            measureFlag = false;
                            statistics_measures();
                            printStatistic();
                            printMeasures();
                            swapRGB();
                            }
                        state = NORMAL;
                        break;
                    }
        }
    }
}
 

