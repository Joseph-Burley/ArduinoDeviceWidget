#include <avr/pgmspace.h>
#include <Arduino_FreeRTOS.h>
#include <Ethernet.h>

void TaskEthernet(void* pvParameters);
TaskHandle_t ethernetHandle = 0;

/*
    Ethernet stuff
*/
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress local(192, 168, 1, 77);
IPAddress server(192, 168, 1, 2);
unsigned int local_port = 9999; //udp only
unsigned int server_port = 9999;

PROGMEM const EthernetClient client;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);

  Serial.println("Setup");
  Serial.print("Size of client: ");
  Serial.println(sizeof(client));

  Ethernet.init(10); //used for spi interface
  Ethernet.begin(mac, local);


  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println(F("No Eth Shield"));
  }
  else
  {

    if (client.connect(server, 9999))
    {
      Serial.println(F("Eth Connected"));
      client.println("Hello");
    }
    else
    {
      Serial.println(F("Eth Failed"));
    }

  }

  BaseType_t eth_succ = xTaskCreate(TaskEthernet,
                                    (const portCHAR*) "Eth",
                                    100,
                                    NULL,
                                    1,
                                    &ethernetHandle);
  if (eth_succ == pdPASS)
  {
    Serial.println("Eth made");
  }
  else
  {
    Serial.println("Eth fail");
  }


//Task schedule automatically started
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
  for(;;)
  {
    Serial.print("Eth Running ");
    Serial.println(num);
    num++;
    client.print("Hello2 ");
    client.println(num);
    vTaskDelay(100);
  }
}
