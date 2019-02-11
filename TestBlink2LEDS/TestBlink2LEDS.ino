#include <Arduino_FreeRTOS.h>

// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );

// the setup function runs once when you press reset or power the board

typedef struct
{
  int pinNum;
  int timeLen;
} blinkParams;

const blinkParams bP1 = {6, 1000};
const blinkParams bP2 = {7, 1500};
const blinkParams bP3 = {5, 500};
void setup() {
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(19200);
  
  Serial.println("Serial Established");
  // Now set up two tasks to run independently.

  
  
  xTaskCreate(
    TaskBlink,  
    (const portCHAR *)"Blink1",   // A name just for humans
    128,  // This stack size can be checked & adjusted by reading the Stack Highwater
    (void*) &bP1,
    2,  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    NULL );

  //Serial.println("Task One Made");

  xTaskCreate(
    TaskBlink
    ,  (const portCHAR *) "Blink2"
    ,  128  // Stack size
    ,  (void*) &bP2
    ,  1  // Priority
    ,  NULL );

    xTaskCreate(
      TaskBlink
      , (const portCHAR*) "Blink3"
      , 128
      , (void*) &bP3
      , 1
      , NULL);

    //Serial.println("Task Two Made");

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters)  // This is a task.
{
  blinkParams* parameters = (blinkParams*) pvParameters;

  //Serial.println("Here");
  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(parameters->pinNum, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(parameters->pinNum, HIGH);   // turn the LED on (HIGH is the voltage level)
    //Serial.print(parameters->pinNum);
    //Serial.println("H");
    vTaskDelay( parameters->timeLen / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(parameters->pinNum, LOW);    // turn the LED off by making the voltage LOW
    //Serial.print(parameters->pinNum);
    //Serial.println("L");
    vTaskDelay( parameters->timeLen / portTICK_PERIOD_MS ); // wait for one second
  }
}
