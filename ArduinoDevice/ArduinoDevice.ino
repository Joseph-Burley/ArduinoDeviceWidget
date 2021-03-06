//#include <SPI.h>
#include <Ethernet.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>

//Declare Three tasks. one for Serial coms, one for Ethernet coms, one for hardware buttons
void TaskSerial(void* pvParameters);
void TaskEthernet(void* pvParameters);
void TaskHardware(void* pvParameters);

TaskHandle_t serialHandle = 0; //Used to verify serial task has been created
TaskHandle_t ethernetHandle = 0;
TaskHandle_t hardwareHandle = 0;

//Three queues. One to Hardware, one to Serial, one to Ethernet
QueueHandle_t xSerialQueue = 0; //For transmiting data to the Serial task
QueueHandle_t xEthernetQueue = 0; //For transmitting data to the Ethernet task
QueueHandle_t xHardwareQueue = 0; //For transmitting data to the hardware task

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

 /*
  * Ethernet stuff
  */
   //byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
   //IPAddress local(192,168,1,77);
   //IPAddress server(192,168,1,2);
   //unsigned int local_port = 9999;
   //unsigned int server_port = 9999;
   //EthernetClient client;
   //volatile EthernetUDP Udp;

void setup() {
  Serial.begin(19200);
  Serial.setTimeout(45); //timeout is 45 ms (3 ticks)


  Serial.println("Setup");
  //first queues are made
  xSerialQueue = xQueueCreate(queueLength, sizeof(message));
  xEthernetQueue = xQueueCreate(queueLength, sizeof(message));
  xHardwareQueue = xQueueCreate(queueLength, sizeof(message));

  if(xSerialQueue == NULL)
  {
    Serial.println(F("Serial Queue Fail"));
  }
  if(xHardwareQueue == NULL)
  {
    Serial.println(F("Hardware Queue Fail"));
  }
  if(xEthernetQueue == NULL)
  {
    Serial.println(F("Ethernet Queue Fail"));
  }
  Serial.println("queues");


  //Ethernet.init(10); //used for spi interface
  //Ethernet.begin(mac,local);

/*
  if(Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println(F("No Eth Shield"));
  }
  else
  {

    if(client->connect(server, 9999))
    {
      Serial.println(F("Eth Connected"));
      client->println("Hello");
      client->stop();
    }
    else
    {
      Serial.println(F("Eth Failed"));
    }

  }
  */



  BaseType_t serial_succ = xTaskCreate(TaskSerial,
                          (const portCHAR*) "Serial",
                          171,
                          NULL,
                          3,
                          &serialHandle);
if(serial_succ == pdPASS)
{
  Serial.println("Serial made");
}
else
{
  Serial.println("Serial fail");
}

  BaseType_t hardware_succ = xTaskCreate(TaskHardware,
              (const portCHAR*) "Hardware",
              96,
              NULL,
              1,
              &hardwareHandle);
if(hardware_succ == pdPASS)
{
  Serial.println("Hardware made");
}
else
{
  Serial.println("Hardware fail");
}


  BaseType_t eth_succ = xTaskCreate(TaskEthernet,
              (const portCHAR*) "Eth",
              100,
              NULL,
              1,
              &ethernetHandle);
if(eth_succ == pdPASS)
{
  Serial.println("Eth made");
}
else
{
  Serial.println("Eth fail");
}


Serial.print((int) serialHandle, HEX);
  Serial.print('\n');
  Serial.print((int) hardwareHandle, HEX);
  Serial.print('\n');
  Serial.print((int) ethernetHandle, HEX);
  Serial.print('\n');

}

void loop() {
  // put your main code here, to run repeatedly:

}

void TaskEthernet(void* pvParameters)
{
  Serial.println("Eth Task Start");
  int num = 0;

  /*
   * This task listens on the Ethernet port
   */
  message EthMessage;
  EthMessage.source = 1;
  EthMessage.opcode = 0;
  EthMessage.data = 0;

  char buff [] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  for(;;)
  {
    Serial.print("Eth Running ");
    Serial.println(num);
    num++;
    //client->println("Hello2");
    vTaskDelay(100);
  }
}


void TaskSerial( void* pvParameters)
{
  Serial.println("Serial Task Start");
  unsigned int max_stack = -1;
  Serial.print((int) serialHandle, HEX);
  Serial.print('\n');
  Serial.print((int) ethernetHandle, HEX);
  Serial.print('\n');
  Serial.print((int) hardwareHandle, HEX);
  Serial.print('\n');

  /*
     This task listens on the serial port and serial queue.
     When a message arrives it validates it before sending it along.
  */
  message SerialMessage;
  SerialMessage.source = 0;
  SerialMessage.opcode = 0;
  SerialMessage.data = 0;

  char buff [] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  for (;;)
  {
    if(buff[0] != '\0')
    {
      for(int i=0; i<20; i++)
      {
        buff[i] = '\0';
      }
    }
    if (Serial.available() > 0)
    {
      Serial.readBytesUntil('\n', buff, 20);
      /*
         This splits the buffer into tokens.
         There should be no more than three tokens.
         The first token should be a single character
         (i.e. token_index[0][1] should be token_index[0][0] + 1)
         The second and third tokens should be 1-3 numeric characters
      */
      int err_no = 0;

      int token_index[20][2];
      for (int i = 0; i < 20; i++)
      {
        for (int j = 0; j < 2; j++)
        {
          token_index[i][j] = -1;
        }
      }

      bool in_token = false;
      int current_token = 0;
      for (int i = 0; i < 20; i++)
      {
        char c = buff[i];
        if (c <= 32 && in_token)
        {
          token_index[current_token][1] = i;
          current_token++;
          in_token = false;
        }
        else if (c > 32 && !in_token)
        {
          token_index[current_token][0] = i;
          in_token = true;
        }
      }

      int token_count = 0;
      for (int i = 0; i < 20; i++)
      {
        if (token_index[i][0] >= 0)
          token_count++;
      }

      //check number of tokens
      if (token_count < 2)
      {
        err_no |= (1 << 0); //bit 0 indicates too few arguments
      }
      else if (token_count > 3)
      {
        err_no |= (1 << 1); //bit 1 indicates too many arguments
      }

      /*
         Check first token.
         Must be a single character. Either 'R' or 'W'
      */

      if (token_count >= 1)
      {
        if (token_index[0][0] == (token_index[0][1] - 1))
        {
          //correct number
          if (buff[token_index[0][0]] == 'R') //indicates Reading
            SerialMessage.opcode |= (1 << 7);
          else if (buff[token_index[0][0]] == 'W') //indicates Writing
            SerialMessage.opcode &= ~(1 << 7);
          else
            err_no |= (1 << 2);
        }
        else
        {
          err_no |= (1 << 2); //bit 2 indicates first token is invalid
        }
      }

      //check if all characters in token 2 are numeric
      //then convert to an integer
      int paramNum = 0;
      bool all_numeric = true;
      if (token_count >= 2)
      {
        for (int i = token_index[1][0]; i < token_index[1][1]; i++)
        {
          if (buff[i] < 48 || buff[i] > 57)
          {
            all_numeric = false;
            err_no |= (1 << 3); //bit 3 indicates second token is invalid
          }
        }

        if (all_numeric)
        {
          int exponent = 0;
          for (int i = token_index[1][1] - 1; i >= token_index[1][0]; i--)
          {
            paramNum += (buff[i] - 48) * pow(10, exponent);
            exponent++;
          }
        }
      }

      //check token 3 the same way as token 2
      int paramDat = 0;
      all_numeric = true;
      if (token_count >= 3)
      {
        for (int i = token_index[2][0]; i < token_index[2][1]; i++)
        {
          if (buff[i] < 48 || buff[i] > 57)
          {
            all_numeric = false;
            err_no |= (1 << 4); //bit 4 indicates third token is invalid
          }
        }

        if (all_numeric)
        {
          int exponent = 0;
          for (int i = token_index[2][1] - 1; i >= token_index[2][0]; i--)
          {
            paramDat += (buff[i] - 48) * pow(10, exponent);
            exponent++;
          }
        }
      }


      SerialMessage.opcode |= paramNum;
      SerialMessage.data = paramDat;


      /*
        There is no other input validation at this time.
        Simply enqueue the message and clear the buffer
      */
      if (err_no == 0)
      {
        Serial.print(uxQueueSpacesAvailable(xHardwareQueue), DEC);
        bool succ = xQueueSend(xHardwareQueue, &SerialMessage, 0);
        if (!succ)
        {
          Serial.println("queue full");
        }

        SerialMessage.opcode = 0;
        SerialMessage.data = 0;
      }
      else
      {
        Serial.print("err_no: ");
        Serial.print(err_no, HEX);
        Serial.print('\n');
      }
    }


    if (xQueueReceive(xSerialQueue, &SerialMessage, 1) == pdPASS)
    {
      Serial.println("Received message");
      Serial.println(SerialMessage.source);
      Serial.print(SerialMessage.opcode, BIN);
      Serial.print('\n');
      Serial.println(SerialMessage.data);
    }



    int temp = uxTaskGetStackHighWaterMark(NULL);
    if (temp < max_stack)
    {
      max_stack = temp;
      Serial.print("Serial Highwater ");
      Serial.println(max_stack);
    }

    vTaskDelay(10);
  }

}

void TaskHardware(void* pvParameters)
{
  Serial.println("Hardware Task Start");
  unsigned int max_stack = -1;
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
      Serial.println("\n\nmessage received");
      Serial.print(HardwareMessage.opcode, BIN);
      Serial.println();
      Serial.print(HardwareMessage.data, BIN);

      //the number of the input or output being referenced
      unsigned int rw = HardwareMessage.opcode & (1 << 7); //MSB of opcode is for r/w select
      unsigned int num = HardwareMessage.opcode & ~(1 << 7); //grab the 7 LSB of opcode

      Serial.println("\n\nInternals");
      Serial.print("rw: ");
      Serial.print(rw, BIN);
      Serial.println();
      Serial.print("num: ");
      Serial.print(num, BIN);
      Serial.println();

      if (rw)
      {
        //read output
        HardwareMessage.data = out_val_curr[num];
        xQueueSend(xSerialQueue, &HardwareMessage, 0);
      }
      else if (!rw)
      {
        //write input
        out_val_next[num] = HardwareMessage.data;
      }
      else
      {
        Serial.println("wat");
      }

      HardwareMessage.source = 0;
      HardwareMessage.opcode = 0;
      HardwareMessage.data = 0;

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

    int temp = uxTaskGetStackHighWaterMark(NULL);
    if (temp < max_stack)
    {
      max_stack = temp;
      Serial.print("Hardware Highwater ");
      Serial.println(max_stack);
    }

    vTaskDelay( 1 );
  }
}
