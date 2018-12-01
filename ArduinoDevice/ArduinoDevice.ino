#include <Arduino_FreeRTOS.h>
#include <queue.h>

//Declare Three tasks. one for Serial coms, one for Ethernet coms, one for hardware buttons
void TaskSerial(void* pvParameters);
//void TaskEthernet(void* pvParameters);
void TaskHardware(void* pvParameters);

TaskHandle_t serialHandle; //Used to verify serial task has been created
TaskHandle_t hardwareHandle;

//Three queues. One to Hardware, one to Serial, one to Ethernet
QueueHandle_t xSerialQueue; //For transmiting data to the Serial task
//QueueHandle_t xEthernetQueue; //For transmitting data to the Ethernet task
QueueHandle_t xHardwareQueue; //For transmitting data to the hardware task

/*
   The queues all have a length of 5 items and a width of 32 bytes
   (1 byte opcode, 1 byte message length, 30 bytes message)
*/
const int queueLength = 5;
const int queueWidth = 32;

void setup() {
  Serial.begin(9600);

  //first queues are made
  xSerialQueue = xQueueCreate(queueLength, queueWidth);
  //xEthernetQueue = xQueueCreate(queueLength, queueWidth);
  xHardwareQueue = xQueueCreate(queueLength, queueWidth);

  

  

  xTaskCreate(TaskSerial, 
              (const portCHAR*) "Serial",
              128,
              NULL,
              2,
              &serialHandle);

  xTaskCreate(TaskHardware,
              (const portCHAR*) "Hardware",
              128,
              NULL,
              1,
              &hardwareHandle);

  
}

void loop() {
  // put your main code here, to run repeatedly:

}


void TaskSerial( void* pvParameters)
{
  Serial.write((int) xSerialQueue );
  Serial.write('\n');
  Serial.write((int) xHardwareQueue);
  Serial.write('\n');
  Serial.write((int) serialHandle);
  Serial.write('\n');
  Serial.write((int) hardwareHandle);
  Serial.write('\n');

  /*
     This task listens on the serial port and serial queue.
     When a message arrives it validates it before sending it along.
  */
  char* messageBuffer [32];

  for (;;)
  {
    
    if (Serial.available() > 0)
    {
      Serial.println('I');
      if (Serial.available() > 32)
      {
        Serial.println("Input too long");
      }
      else
      {
        Serial.println("here");
        //read message into buffer
        Serial.readBytes(*messageBuffer, 32); //<--- The source of the bug is probably here. readBytes may be blocking execution

        /*
           There is no other input validation at this time.
           Simply enqueue the message and clear the buffer
        */
        Serial.println("free space: " + uxQueueSpacesAvailable(xHardwareQueue));
        bool succ = xQueueSend(xHardwareQueue, messageBuffer, 0);
        if(!succ)
        {
          Serial.println("queue full");
        }
        *messageBuffer = 0;
      }
    }

    if (xQueueReceive(xSerialQueue, messageBuffer, 1) == pdPASS)
    {
      Serial.println("Received message");
      Serial.write((char*) messageBuffer, 32);
      *messageBuffer = 0;
    }
  }

}

void TaskHardware(void* pvParameters)
{
  Serial.println("Hello");
  /*
     This task reads pins and sets outputs.
     It also reads from the Hardware queue to apply software inputs
     or respond to requests for data.
  */

  //I don't think the queue handles need to be passed into the parameters

  //define output pin numbers
  const int num_out = 3;
  const int out_pins[] = {7, 6, 5};
  bool out_val_curr[num_out];
  bool out_val_next[num_out];

  //a char buffer to store data
  char* messBuff [32];

  //set pins to output mode and set low
  for(int i=0; i<num_out; i++)
  {
    out_val_curr[i] = HIGH;
    out_val_next[i] = HIGH;
    pinMode(out_pins[i], OUTPUT);
    digitalWrite(out_pins[i], HIGH);
  }

  pinMode(8, INPUT);

  for (;;)
  {
    //for now only reads from queue
    //by default 1 tick is 15ms
    if (xQueueReceive(xHardwareQueue, messBuff, 0) != pdPASS)
    {
      //normally here is where I would check for button inputs
      if(digitalRead(8))
      {
        out_val_next[0] = !out_val_next[0];
      }
    }
    else
    {
      /*
         Check bit 5 of first byte.
         If 1, is applying input
         If 0, is reading current output
      */
      
      //the number of the input or output being referenced
      byte num = *messBuff[0] & 0x1F;

      if (*messBuff[0] & (1 << 5))
      {
        /* Input
           mask 5 LSB to determine input number to change
           if greater than total number of inputs reject it
        */
        //build the value being input
        unsigned long val = 0;
        for (byte i = *messBuff[1]; i > 0; i--)
        {
          val += *messBuff[i + 2] << (i * 8);
        }
        bool val_bool = (bool) val;
        if (num < num_out)
        {
          out_val_next[num] = val_bool;
        }
      }
      else
      {
        /*
         * Output
         * A read request will never be more than 1 byte long
         */

        *messBuff[1] = 1;
        *messBuff[2] = out_val_curr[num];
        xQueueSend(xSerialQueue, messBuff, 1);
      }

      
    }

    /*
     * Check out_val_curr against our_val_prev for differences
     * If there are differences, apply changes to pins
     * And write new values from curr to prev
     */

    bool sent = false;
    for(int i=0; i<num_out; i++)
    {
      if(out_val_curr[i] != out_val_next[i])
      {
        sent = true;
        digitalWrite(out_pins[i], out_val_next[i]);
        out_val_curr[i] = out_val_next[i];
        
      }
    }

    if(sent)
    {
     *messBuff[0] = 'f'; 
    }

    *messBuff = 0;
    vTaskDelay( 1 );
  }
}
