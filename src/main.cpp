#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h> // add the FreeRTOS functions for Semaphores (or Flags).

float PHValue_1;

// Declare a mutex Semaphore Handle which we will use to manage the Serial Port.
// It will be used to ensure only one Task is accessing this resource at any time.
SemaphoreHandle_t xSerialSemaphore;

//define task handles
TaskHandle_t TaskBlink_Handler;

// define two Tasks for DigitalRead & AnalogRead
// void TaskDigitalRead(void *pvParameters);
void TaskAnalogPHRead(void *pvParameters);
void TaskSerialStatusPrint(void *pvParameters);
void TaskSerialReadCli(void *pvParameters);

// the setup function runs once when you press reset or power the board
void setup()
{

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  // Semaphores are useful to stop a Task proceeding, where it should be paused to wait,
  // because it is sharing a resource, such as the Serial port.
  // Semaphores should only be used whilst the scheduler is running, but we can set it up here.
  if (xSerialSemaphore == NULL) // Check to confirm that the Serial Semaphore has not already been created.
  {
    xSerialSemaphore = xSemaphoreCreateMutex(); // Create a mutex semaphore we will use to manage the Serial Port
    if ((xSerialSemaphore) != NULL)
      xSemaphoreGive((xSerialSemaphore)); // Make the Serial Port available for use, by "Giving" the Semaphore.
  }

  xTaskCreate(
      TaskAnalogPHRead, "TaskAnalogPHRead" // A name just for humans
      ,
      128 // Stack size
      ,
      NULL // Parameters for the task
      ,
      1 // Priority
      ,
      NULL); // Task Handle

  xTaskCreate(
      TaskSerialReadCli, "TaskSerialReadCli" // A name just for humans
      ,
      128 // Stack size
      ,
      NULL // Parameters for the task
      ,
      2 // Priority
      ,
      NULL); // Task Handle
  
   xTaskCreate(
      TaskSerialStatusPrint, "TaskSerialStatusPrint" // A name just for humans
      ,
      128 // Stack size
      ,
      NULL // Parameters for the task
      ,
      3 // Priority
      ,
      &TaskBlink_Handler); // Task Handle

  // Now the Task scheduler, which takes over control of scheduling individual Tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskSerialReadCli(void *pvParameters)
{
  (void)pvParameters;

  bool isDebug = false;

  for (;;) // A Task shall never return or exit.
  {

    if (!isDebug) {
        vTaskSuspend(TaskBlink_Handler); 
    }

    if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE)
    {
      // We were able to obtain or "Take" the semaphore and can now access the shared resource.
      // We want to have the Serial Port for us alone, as it takes some time to print,
      // so we don't want it getting stolen during the middle of a conversion.
      // print out the value you read:
      if (Serial.available() > 0 ){
        String received = Serial.readStringUntil('\n');
        Serial.println(received);
        if (received == "DEBUG|1") {
            isDebug = true;
            vTaskResume(TaskBlink_Handler);
        } else if (received == "DEBUG|0")
        {
            vTaskSuspend(TaskBlink_Handler); 
        }
        
      }

      xSemaphoreGive(xSerialSemaphore); // Now free or "Give" the Serial Port for others.
    }
    vTaskDelay(1); // one tick delay (15ms) in between reads for stability
  }
}

void TaskSerialStatusPrint(void *pvParameters)
{
  (void)pvParameters;
  for (;;) // A Task shall never return or exit.
  {

    // if (!isDebug) {
    //   continue;
    // }
 
    // See if we can obtain or "Take" the Serial Semaphore.
    // If the semaphore is not available, wait 5 ticks of the Scheduler to see if it becomes free.
    if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE)
    {
      // We were able to obtain or "Take" the semaphore and can now access the shared resource.
      // We want to have the Serial Port for us alone, as it takes some time to print,
      // so we don't want it getting stolen during the middle of a conversion.
      // print out the value you read:
      Serial.print("$<PH:");
      Serial.print(PHValue_1, 3);
      Serial.print("?");
      Serial.println(">&");

      xSemaphoreGive(xSerialSemaphore); // Now free or "Give" the Serial Port for others.
    }

    vTaskDelay(1); // one tick delay (15ms) in between reads for stability
  }
}

void TaskAnalogPHRead(void *pvParameters __attribute__((unused))) // This is a Task.
{
  int buf[10], temp;
  unsigned long int avgValue;

  unsigned short initialCounter;

  for (;;)
  {
    // read the input on analog pin 0:
    int sensorValue = analogRead(A0);

    buf[0] = sensorValue;

    initialCounter++;

    // Getting at least sample number of readings
    if (initialCounter < sizeof(buf))
    {
      continue;
    }

    int beginning = buf[0];

    memmove(buf, &buf[1], sizeof(buf) - sizeof(int));

    buf[9] = beginning;

    for (int i = 0; i < 9; i++) // sort the analog from small to large
    {
      for (int j = i + 1; j < 10; j++)
      {
        if (buf[i] > buf[j])
        {
          temp = buf[i];
          buf[i] = buf[j];
          buf[j] = temp;
        }
      }
    }

    avgValue = 0;
    for (int i = 2; i < 8; i++) // take the average value of 6 center sample
      avgValue += buf[i];
    float phValue = (float)avgValue * 5.0 / 1024 / 6; // convert the analog into millivolt
    PHValue_1 = 3.5 * phValue;                          // convert the millivolt into pH value
  }
}