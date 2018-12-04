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

//message struct for passing messages between tasks
typedef struct
{
  int source;
  int opcode;
  int data;
} message;

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(45); //timeout is 45 ms (3 ticks)

  //first queues are made
  xSerialQueue = xQueueCreate(queueLength, sizeof(message));
  //xEthernetQueue = xQueueCreate(queueLength, queueWidth);
  xHardwareQueue = xQueueCreate(queueLength, sizeof(message));





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
  Serial.print((int) xSerialQueue, HEX);
  Serial.write('\n');
  Serial.print((int) xHardwareQueue, HEX);
  Serial.write('\n');
  Serial.print((int) serialHandle, HEX);
  Serial.write('\n');
  Serial.print((int) hardwareHandle, HEX);
  Serial.write('\n');

  int max_stack = 0;

  /*
     This task listens on the serial port and serial queue.
     When a message arrives it validates it before sending it along.
  */
  message SerialMessage;
  SerialMessage.source = 0;
  SerialMessage.opcode = 0;
  SerialMessage.data = 0;

  String tempMess;
  tempMess.reserve(20);

  for (;;)
  {

    if (Serial.available() > 0)
    {
      while(Serial.available())
      {
        tempMess += Serial.read();
      }
      tempMess.trim();
      int firstSpace = tempMess.indexOf(' '); //if this is -1 the string has only one argument and is invalid
      if (firstSpace == -1)
      {
        Serial.println(F("Invalid Command: Too few arguments"));
        break;
      }
      int lastSpace = tempMess.indexOf(' ', firstSpace); //if this is -1 the string has only two arguments and must be a read op
      String paramNumArg = tempMess.substring(firstSpace, lastSpace); //0-127 this is the number of the input or output
      String paramDatArg = tempMess.substring(lastSpace); //if output, then is unused. if input, is the value to write

      int paramNum = paramNumArg.toInt();
      int paramDat = paramDatArg.toInt();

      if (tempMess.charAt(0) == 'R') //user is reading from output
      {
        SerialMessage.opcode |= (1 << 7); //set bit 7
      }
      else if (tempMess.charAt(0) == 'W') //user is writing to input
      {
        SerialMessage.opcode &= !(1 << 7); //reset bit 7
      }
      else
      {
        Serial.println(F("Invalid Command: Must be R/W"));
        break;
      }

      SerialMessage.opcode |= paramNum;
      SerialMessage.data = paramDat;


      /*
        There is no other input validation at this time.
        Simply enqueue the message and clear the buffer
      */
      Serial.print(uxQueueSpacesAvailable(xHardwareQueue), DEC);
      bool succ = xQueueSend(xHardwareQueue, &SerialMessage, 0);
      if (!succ)
      {
        Serial.println("queue full");
      }
    }


    if (xQueueReceive(xSerialQueue,&SerialMessage, 1) == pdPASS)
    {
      Serial.println("Received message");
      Serial.println(SerialMessage.source);
      Serial.println(SerialMessage.opcode);
      Serial.println(SerialMessage.data);
    }

    int temp = uxTaskGetStackHighWaterMark(NULL);
    if (temp > max_stack)
    {
      max_stack = temp;
      Serial.write(max_stack);
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
  const unsigned int num_out = 3;
  const unsigned int out_pins[] = {7, 6, 5};
  bool out_val_curr[num_out];
  bool out_val_next[num_out];

  //a char buffer to store data
  message HardwareMessage;
  HardwareMessage.source = 0;
  HardwareMessage.opcode = 0;
  HardwareMessage.data = 0;

  //set pins to output mode and set low
  for (int i = 0; i < num_out; i++)
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
    if (xQueueReceive(xHardwareQueue, &HardwareMessage, 1) != pdPASS)
    {
      //normally here is where I would check for button inputs
      if (digitalRead(8))
      {
        out_val_next[2] = !out_val_next[2];
      }
    }
    else //if  message on hardware queue
    {
      /*
         Check bit 5 of first byte.
         If 1, is applying input
         If 0, is reading current output
      */
      Serial.print("message received");

      //the number of the input or output being referenced
      unsigned int rw = HardwareMessage.opcode & (1 << 7); //MSB of opcode is for r/w select
      unsigned int num = HardwareMessage.opcode & !(1 << 7); //grab the 7 LSB of opcode

      if(rw == 1)
      {
        //read output
        HardwareMessage.data = out_val_curr[num];
        xQueueSend(xSerialQueue, &HardwareMessage, 0);
      }
      else if(rw == 0)
      {
        //write input
        out_val_next[num] = HardwareMessage.data;
      }
      else
      {
        Serial.println("wat");
      }


    }

    /*
       Check out_val_curr against our_val_prev for differences
       If there are differences, apply changes to pins
       And write new values from curr to prev
    */

    for (int i = 0; i < num_out; i++)
    {
      if (out_val_curr[i] != out_val_next[i])
      {
        digitalWrite(out_pins[i], out_val_next[i]);
        out_val_curr[i] = out_val_next[i];

      }
    }
    vTaskDelay( 1 );
  }
}
