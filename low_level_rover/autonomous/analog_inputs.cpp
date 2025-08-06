#include <Arduino.h>
#include "analog_inputs.h"
/* MACROS -------------------------------------------------------------------------- */
#define NUM_PINS  3 // Currently Reading 2 Analog Pins
#define LUT_SIZE 11

/* DATA TYPE DEFINITION  ----------------------------------------------------------- */
typedef struct{
  uint8_t Pin;
  long int Dvalue;
  long int dist;
} analog_pin;



analog_sense_t Sensor[] ={
  {A0,0},
  {A1,0},
  {A2,0}
};
// typedef struct{
//   long int x;    // ADC value for the corresponding distance  (mm)
//   long int m;    // slope between current ADC value and the next one 
//   long int mm;   // distance in mm from the sensor
// } LUT;

// /* VARIABLES  ---------------------------------------------------------------------- */
// LUT values[LUT_SIZE] = {
//   {108, 17653, 800},  // slope: 17.2414
//   {113,  7475, 700},  // slope: 7.2988
//   {127, 10129, 600},  // slope: 9.8911
//   {138,  4212, 500},  // slope: 4.1126
//   {162,  2425, 400},  // slope: 2.3689
//   {204,  1281, 300},  // slope: 1.2508
//   {283,   463, 200},  // slope: 0.4528
//   {504,   193, 100},  // slope: 0.1885
//   {557,   190,  90},  // slope: 0.1861
//   {610,   183,  80},  // slope: 0.1792
//   {667,     0,  70}
// };

// analog_pin A_Pins[] = {
//   {A0, 0, 0},
//   {A1, 0, 0},
//   {A2, 0, 0},
//   // Add more pins
// }; 
// int pin = 0;
/* FUNCTIONS ---------------------------------------------------------------------- */

/*
* Finds in which interval the digital value belongs to
*   If it is lower than the starting value, -1 is returned
*   If is higher than the last value, the last value is returned
*   Else, returns the lower value of the interval to which it belongs to
*/
// int find_interval(long int target) {
//     //menor que o primeiro
//     if (target < values[0].x) 
//         return -1; // Fora da tabela (abaixo)
    
//     // maior que o último
//     if (target >= values[LUT_SIZE-1].x) 
//         return LUT_SIZE-1; 
  
//     // Procura o long intervalo correto
//     for (int i = 0; i < LUT_SIZE - 1; i++) {
//         if (target >= values[i].x && target < values[i+1].x) 
//             return i; 
//     }   
//     return -1; 
// }
/*
* Gives the measured distance in mm
*/
void distance_check(void) {

  digitalWrite(8, 1);
  Sensor[0].raw = analogRead(Sensor[0].pin);
  delay(1);

  Sensor[1].raw = analogRead(Sensor[1].pin);
   delay(1);

  Sensor[2].raw = analogRead(Sensor[2].pin);

  
}

//   analog_pin* eg = &A_Pins[pin];
//   eg->Dvalue = analogRead(eg->Pin);
//   long int item = find_interval(eg->Dvalue);

//   //if(item != -1){
//     /*
//     * (y - y0)/(x - x0) = m
//     */
//     long int dx = eg->Dvalue - values[item].x;
//     long int dy = -values[item].m * dx;
//     eg->dist = (values[item].mm + (dy>>10));  

//     /*
//     Serial.print("    Analog:");
//     Serial.print(eg->Dvalue);
//     Serial.print("  dx: ");
//     Serial.print(dx);
//     Serial.print("  m: ");
//     Serial.print(values[item].m);
//     Serial.print("  dy: ");
//     Serial.print(dy); 
//     */
//     /*
//     Serial.print(pin);
//     Serial.print(" ");
//     Serial.println(eg->dist);
//     */
//     //long int info [] = {(long int)pin, eg->dist};
//     //send_to_rasp(IR, info, 2);

//     pin = (pin + 1) % 3;
//   //}
// }

