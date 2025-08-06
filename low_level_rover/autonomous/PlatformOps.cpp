#include "PlatformOps.h"

#include "bmi323.h"
#include "analog_inputs.h"
#include "motors_rvr.h"

extern analog_sense_t Sensor[];
extern bmi323_data_t sensor_data;

const unsigned long DefaultFrequencyMS = 1000;
unsigned long SampleFrequency_MS;
unsigned long LastStampReceivingFrame;
const unsigned long ConsiderMotion = 3;
const Frame Stop = { CMD_STOP,"stop",2,0,0};



//bool state = false;
bool SampleSensors = false;

// ISR TIMER 1
ISR(TIMER1_COMPA_vect) {
  SampleSensors = true;    // set flag to perform sampling
}

void timer1_setup (void){
  cli(); // desabilita interrupções globais

  TCCR1A = 0; // Modo normal
  TCCR1B = 0;

  // Modo CTC
  TCCR1B |= (1 << WGM12);
  // Prescaler 1024
  TCCR1B |= (1 << CS12) | (1 << CS10);

  // 128 ms com clock de 16MHz: OCR1A = (F_CPU / Prescaler) * Tempo - 1
  // OCR1A = (16.000.000 / 1024) * 0.128 - 1 ≈ 1999
  //OCR1A = 1999;
  OCR1A = 15624;

  // Habilita interrupção
  TIMSK1 |= (1 << OCIE1A);

  sei(); // habilita interrupções globais
}


void timer1_start(void)
{
    unsigned int  ms2reg;
    ms2reg = DefaultFrequencyMS;
    SampleFrequency_MS = DefaultFrequencyMS;
    ms2reg = 15625 * ms2reg / 1000 - 1;
}

void timer1_start(unsigned int time_ms)
{
    unsigned int  ms2reg;
    ms2reg = time_ms;
    SampleFrequency_MS = time_ms;
    ms2reg = 15625 * ms2reg / 1000 - 1;
    OCR1A = (uint16_t)ms2reg;
}

void timer1_stop(void)
{
    if (motors_in_motion())
        parser_motors(&Stop);
}
// --- Simulated sensor reading ---
void readSensors() {


    distance_check();
    //digitalWrite(8, HIGH);
    bmi323_read_data_burst();
    //digitalWrite(8, LOW);


    Serial.print("<ir,3,");
    Serial.print((Sensor[0].raw));
    Serial.print(",");
    Serial.print((Sensor[1].raw));
    Serial.print(",");
    Serial.print((Sensor[2].raw));
    Serial.print("><acc,3,");
    Serial.print(sensor_data.acc_x);
    Serial.print(",");
    Serial.print(sensor_data.acc_y);
    Serial.print(",");
    Serial.print(sensor_data.acc_z);
    Serial.print("><gyr,3,");
    Serial.print(sensor_data.gyr_x);
    Serial.print(",");
    Serial.print(sensor_data.gyr_y);
    Serial.print(",");
    Serial.print(sensor_data.gyr_z);
    Serial.print("><temp,1,");
    Serial.print(sensor_data.temperature);
    Serial.print("><time,1,");
    Serial.print(sensor_data.sensor_time);
    Serial.println(">");


}

bool handleSerialInput(String & serialIn) {
    bool frameReady = false;
    //Serial.println("handle Serial");
    if (Serial.available()) {

        char c = Serial.read();
        serialIn += c;
        //Serial.print(c);
        if (c == '\n') {
            //Serial.println(serialIn);
            frameReady = true;
        }
    }
    return frameReady;

}

void check_receiving_speed_frames(void){
    unsigned long time_now;
   
    if (motors_in_motion()) {
        time_now = millis();
        if (time_now > ConsiderMotion * SampleFrequency_MS + LastStampReceivingFrame) {
            parser_motors(&Stop);
        }
    }
}

void ack_receiving_speed_frames(void) {
    LastStampReceivingFrame = millis();
}