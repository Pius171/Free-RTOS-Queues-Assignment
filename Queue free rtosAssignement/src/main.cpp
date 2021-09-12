#include <Arduino.h>

//use only one core
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

// settings

static const uint8_t msg_queue_len = 5; // maximum number of queues our queue can store

// create our queue variable as a global variable
// so both of our tasks can use it
// not our 'QueueHandle_t' is a struct
static QueueHandle_t msg_queue1;
static QueueHandle_t msg_queue2;

//tasks
void printMessages(void *parameters)
{
  String response_queue2; // this is where i would store the messages in queue2,
                          // I am sending integers values so it is an 'int'

  while (1)
  {

    // check if there is a message in the queue
    if (xQueueReceive(msg_queue2, (void *)&response_queue2, 0) == pdTRUE) //returns pdTRUE if an item was successfully received from the queue,otherwise pdFALSE.
                                                                          // the message is then stored in the address of item(&item) and is converted to a void
                                                                          // pointer, while '0' is the number of TICKS(delay) to wait for the message to arrive
    {
      Serial.println(response_queue2); // print the item to the serial monitor
    }
    String msg = Serial.readString();
    String Delay = "";
    int delay_int = 0;
    if (msg.length() > 0)
    {
      Serial.println(msg);

      if (msg.startsWith("delay"))
      {

        for (int i = 0; i < msg.length(); i++)
        {
          if (isDigit(msg[i]))
          {
            Delay += msg[i];
          }
        }
        delay_int = Delay.toInt();
        //Serial.println(Delay);

        // try to add items to  queue1 for 10 ticks, fail if queue is full
        if (xQueueSend(msg_queue1, (void *)&delay_int, 10) != pdTRUE)
        {
          Serial.println("Queue Full");
        }
      }
    }
    //Serial.println(item); // print the value in item whether the message is received or not
    //wait for 1 sec before trying to read
    //vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void BlinkLed(void *parameters)
{
  int led_delay = 2000;
  long blink_count =0;
  int blink_count2=0;
  const String message = "Blinked";
  //String response_queue1="";
  while (1)
  {
    if (xQueueReceive(msg_queue1, (void *)&led_delay, 0) == pdTRUE) //returns pdTRUE if an item was successfully received from the queue,otherwise pdFALSE.
                                                                    // the message is then stored in the address of item(&item) and is converted to a void
                                                                    // pointer, while '0' is the number of TICKS(delay) to wait for the message to arrive
    {
      Serial.println(led_delay); // print the item to the serial monitor
    }
    //blink led
    digitalWrite(4,1);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    digitalWrite(4,0); //turn off led
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    blink_count++; blink_count2++;
    String blink_count2_string = String(blink_count2);
    if(blink_count==100){ // send message "Blinked" everytime the led blinks 100 times
      blink_count=0;

      if (xQueueSend(msg_queue2, (void *)&message, 10) != pdTRUE)
        {
          Serial.println("Queue2 Full");
        }
    }
   if (xQueueSend(msg_queue2, (void *)&blink_count2_string, 10) != pdTRUE)
        {
          Serial.println("Queue2 Full");
        }

  }
}
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(4,OUTPUT); // pin for our LED
  
  //wait one sec so we dont miss serial output
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("--Free Rtos Queue Assignment--");

  // create a queue
  msg_queue1 = xQueueCreate(msg_queue_len, sizeof(int)); // create a queue takes two variables the max length of queue and
                                                         // The size, in bytes, required to hold each item in the queue.

  msg_queue2 = xQueueCreate(msg_queue_len, sizeof(String));

  xTaskCreatePinnedToCore(
      printMessages // function to run
      ,
      "printMessages" // task name
      ,
      1024 // Stack size
      ,
      NULL, 
      1 // Priority
      ,
      NULL, 
      
      ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
      BlinkLed // function to run
      ,
      "BlinkLed" // task name
      ,
      1024*2 // Stack size
      ,
      NULL, 
      1 // Priority
      ,
      NULL, 
      ARDUINO_RUNNING_CORE);
}

void loop()
{
  // put your main code here, to run repeatedly:
}