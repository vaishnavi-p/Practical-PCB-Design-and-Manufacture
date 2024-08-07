// vrm characterizer board
#include <Wire.h>
#include <Adafruit_MCP4725.h> 
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads; 
Adafruit_MCP4725 dac;
float R_sense = 10.0; 
long itime_on_msec = 100;
//current sensor
//on time for taking measurements
long itime_off_msec = itime_on_msec * 10; int iCounter_off = 0;
int iCounter_on = 0;
float v_divider = 5000.0 / 15000.0; 
float DAC_ADU_per_v = 4095.0 / 5.0;
int V_DAC_ADU;
int I_DAC_ADU;
float I_A = 0.0; 
long itime_stop_usec;
float ADC_V_per_ADU = 0.125 * 1e-3; 
float V_VRM_on_v;
float V_VRM_off_v;

float I_sense_on_A;

float I_sense_off_A;
float I_max_A = 0.25;

int npts = 20; //number of points to measure 
float I_step_A = I_max_A / npts; //step current change
float I_load_A; // the measured current load 
float V_VRM_thevenin_v;
float V_VRM_loaded_v;
float R_thevenin;
int i;

void setup() {
Serial.begin(115200);
dac.begin(0x60); // address is either 0x60, 0x61, 0x62,0x63, 0x64 or 0x65 dac.setVoltage(0, false); //sets the output current to 0 initially

// ads.setGain(GAIN_TWOTHIRDS); // 2/3x gain +/- 6.144V 1 bit = 3mV 0.1875mV (default)
ads.setGain(GAIN_ONE); // 1x gain +/- 4.096V 1 bit = 2mV
ads.begin(0x48); // note- you can put the address of the ADS111 here if needed
ads.setDataRate(RATE_ADS1115_860SPS);// sets the ADS1115 for higher speed
}

void loop() {
for (i = 1; i <= npts; i++) {
I_A = i * I_step_A;
dac.setVoltage(0, false); //sets the output current
func_meas_off();
func_meas_on();
dac.setVoltage(0, false); //sets the output current
I_load_A = I_sense_on_A - I_sense_off_A; //load current 
V_VRM_thevenin_v = V_VRM_off_v;
V_VRM_loaded_v = V_VRM_on_v;
R_thevenin = (V_VRM_thevenin_v - V_VRM_loaded_v) / I_load_A;
//if (V_VRM_loaded_v < 0.75 * V_VRM_thevenin_v) i = npts; //stops the ramping
Serial.print(i);
Serial.print(", "); 
Serial.print(I_load_A * 1e3, 3); 
Serial.print(", "); 
Serial.print(V_VRM_thevenin_v, 4); 
Serial.print(", "); 
Serial.print(V_VRM_loaded_v, 4);
Serial.print(", ");
Serial.println(R_thevenin, 4);
}
Serial.println("done"); 
delay(30000);
}

void func_meas_off()
{
dac.setVoltage(0, false); //sets the output current
iCounter_off = 0;
V_VRM_off_v = 0.0;
I_sense_off_A = 0.0;
//starting the current counter //initialize the VRM voltage averager
// initialize the current averager
itime_stop_usec = micros() + itime_off_msec * 1000; // stop time
  float vrm_off;
  float isense_off;
while (micros() <= itime_stop_usec) {

  vrm_off = ads.readADC_Differential_0_1();
  isense_off = ads.readADC_Differential_2_3();
    //Serial.print(vrm_off, 4); 
      //Serial.print("isense_off");
  //Serial.println(isense_off, 4); 
V_VRM_off_v = (( vrm_off* ADC_V_per_ADU) / v_divider) + V_VRM_off_v;
I_sense_off_A = (( isense_off* ADC_V_per_ADU) / R_sense) + I_sense_off_A;
      iCounter_off++;
}
V_VRM_off_v = V_VRM_off_v / iCounter_off;
I_sense_off_A = I_sense_off_A / iCounter_off;
// Serial.print(iCounter_off);Serial.print(", ");
// Serial.print(I_sense_off_A * 1e3, 4); 
//Serial.print(", "); 
// Serial.println(V_VRM_off_v, 4);
}

void func_meas_on(){ //now turn on the current
I_DAC_ADU = I_A * R_sense * DAC_ADU_per_v; 
dac.setVoltage(I_DAC_ADU, false); //sets the output current
iCounter_on = 0;
V_VRM_on_v = 0.0; //initialize the VRM voltage averager 
I_sense_on_A = 0.00; // initialize the current averager 
itime_stop_usec = micros() + itime_on_msec * 1000; // stop time
float vrm_on;
float isense_on;
while (micros() <= itime_stop_usec) {

    vrm_on = ads.readADC_Differential_0_1();
  isense_on = ads.readADC_Differential_2_3();
    //Serial.print(vrm_on, 4); 
  //Serial.print("isense_on");
  //Serial.println(isense_on, 4); 
  
V_VRM_on_v = ((vrm_on * ADC_V_per_ADU) / v_divider) + V_VRM_on_v;

I_sense_on_A = ((isense_on * ADC_V_per_ADU) / R_sense) + I_sense_on_A;
 iCounter_on++;
}
dac.setVoltage(0, false); //sets the output current to zero
V_VRM_on_v = V_VRM_on_v / iCounter_on; 
I_sense_on_A = I_sense_on_A / iCounter_on;
// Serial.print(iCounter_on);Serial.print(", ");
// Serial.print(I_sense_on_A * 1e3, 4);
//Serial.print(", "); // Serial.println(V_VRM_on_v, 4);
}
