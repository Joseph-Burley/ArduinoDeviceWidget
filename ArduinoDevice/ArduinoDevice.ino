#include <Arduino_FreeRTOS.h>
#include <queue.h>

//Declare Three tasks. one for Serial coms, one for Ethernet coms, one for hardware buttons
void TaskSerial(void* pvParameters);
void TaskEthernet(void* pvParameters);
void TaskHardware(void* pvParameters);

//Three queues. One to Hardware, one to Serial, one to Ethernet
QueueHandle_t xSerialQueue; //For transmiting data to the Serial task
QueueHandle_t xEthernetQueue; //For transmitting data to the Ethernet task
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
  xEthernetQueue = xQueueCreate(queueLength, queueWidth);
  xHardwareQueue = xQueueCreate(queueLength, queueWidth);

  if (xSerialQueue == NULL || xEthernetQueue == NULL || xHardwareQueue == NULL)
  {
    Serial.println("A queue could not be created");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}


void TaskSerial( void* pvParameters)
{
  /*
     This task listens on the serial port and serial queue.
     When a message arrives it validates it before sending it along.
  */
  char* messageBuffer [32];

  for (;;)
  {
    if (Serial.available() > 0)
    {
      if (Serial.available() > 32)
      {
        Serial.println("Input too long");
      }
      else
      {
        //read message into buffer
        Serial.readBytes(*messageBuffer, 32);

        /*
           There is no other input validation at this time.
           Simply enqueue the message and clear the buffer
        */
        xQueueSend(xHardwareQueue, messageBuffer, 0);
        *messageBuffer = 0;
      }
    }

    if (xQueueReceive(xSerialQueue, messageBuffer, 0) == pdPASS)
    {
      Serial.println("Received message");
      Serial.write((char*) messageBuffer, 32);
      *messageBuffer = 0;
    }
  }

}

void TaskHardware(void* pvParameters)
{
  /*
   * This task reads pins and sets outputs.
   * It also reads from the Hardware queue to apply software inputs
   * or respond to requests for data.
   */
}
