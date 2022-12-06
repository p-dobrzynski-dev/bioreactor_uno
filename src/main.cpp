#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h> // add the FreeRTOS functions for Semaphores (or Flags).

// Reading variables
float PHValue;
float TemperatureValue;
float GravValue;

// Pumps
int PUMP_1_Value;
int PUMP_2_Value;
int PUMP_3_Value;
int PUMP_4_Value;

// Debug flags
bool isFastDebug = false;
bool isPumpDebug = false;

enum ERROR
{
  OK = 0,
  BAD_SYNTAX = 1,
  INVALID_COMMAND = 2,
  INVALID_PARAMETER = 3,
};

const String CommandPrefix = "CMD";

const String DebugFastCMD = "DEBUG_FAST";
const String DebugPumpCMD = "DEBUG_PUMP";

ERROR errorCode; // Error base object

// Declare a mutex Semaphore Handle which we will use to manage the Serial Port.
// It will be used to ensure only one Task is accessing this resource at any time.
SemaphoreHandle_t xSerialSemaphore;

// define two Tasks for DigitalRead & AnalogRead
// void TaskDigitalRead(void *pvParameters);
void TaskAnalogPHRead(void *pvParameters);
void TaskSerialReadWriteTerminal(void *pvParameters);

// the setup function runs once when you press reset or power the board
void setup()
{
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  Serial.println("CONNECTED");

  errorCode = OK;

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
      TaskSerialReadWriteTerminal, "TaskSerialReadWriteTerminal" // A name just for humans
      ,
      256 // Stack size
      ,
      NULL // Parameters for the task
      ,
      2 // Priority
      ,
      NULL); // Task Handle

  // Now the Task scheduler, which takes over control of scheduling individual Tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Helpers -------------------*/
/*--------------------------------------------------*/

#pragma region terminal_helpers

void PrintInfo()
{
  Serial.println();
  Serial.println("/*--------------------------------------------------*/");
  Serial.println("/*------------------ BIO REACTOR -------------------*/");
  Serial.println("/*---------------------- INFO ----------------------*/");
  Serial.println();
  Serial.println("DEBUG ...");
}

#pragma endregion

#pragma region commands_reponses_helper

void SendOk(String message)
{
  errorCode = OK;
  Serial.println(message + "|ERROR:" + errorCode);
}

void SendSyntaxError(String message)
{
  errorCode = BAD_SYNTAX;
  Serial.println(message + "|ERROR:" + errorCode);
}

void SendInvalidCommand(String message)
{
  errorCode = INVALID_COMMAND;
  Serial.println(message + "|ERROR:" + errorCode);
}

void SendInvalidParameters(String message)
{
  errorCode = INVALID_PARAMETER;
  Serial.println(message + "|ERROR:" + errorCode);
}

#pragma endregion

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskSerialReadWriteTerminal(void *pvParameters)
{
  (void)pvParameters;

  for (;;) // A Task shall never return or exit.
  {

    if (xSemaphoreTake(xSerialSemaphore, (TickType_t)5) == pdTRUE)
    {
      // We were able to obtain or "Take" the semaphore and can now access the shared resource.
      // We want to have the Serial Port for us alone, as it takes some time to print,
      // so we don't want it getting stolen during the middle of a conversion.
      // print out the value you read:

      if (Serial.available() > 0)
      {
        String baseReceivedMessage = Serial.readStringUntil('\n'); // Reading line until endline mark

        String receivedMessage = baseReceivedMessage;

        String receivedArray[4];
        int itemCount = 0;

        // Sltting message to string array
        while (receivedMessage.length() > 0)
        {
          int index = receivedMessage.indexOf(',');
          if (index == -1) // No char found
          {
            receivedArray[itemCount++] = receivedMessage;
            break;
          }
          else
          {
            receivedArray[itemCount++] = receivedMessage.substring(0, index);
            receivedMessage = receivedMessage.substring(index + 1);
          }
        }

        if (baseReceivedMessage == "?")
        {
          PrintInfo();
        }
        else if (receivedArray[0] == "" || receivedArray[0] == NULL || receivedArray[0] == baseReceivedMessage)
        {
          // BAD_SYNTAX
          SendSyntaxError(baseReceivedMessage);
        }
        else
        {
          if (receivedArray[0] == CommandPrefix)
          {
            // COMMAND: DEBUG_FAST and DEBUG_PUMP
            if (receivedArray[1] == DebugFastCMD || receivedArray[1] == DebugPumpCMD)
            {
              if (receivedArray[3] != NULL || receivedArray[4] != NULL)
              {
                // INVALID_PARAMETER
                SendInvalidParameters(baseReceivedMessage);
              }
              else
              {
                if (receivedArray[2] == "0")
                {
                  if (receivedArray[1] == DebugFastCMD)
                  {
                    isFastDebug = false;
                  }
                  else
                  {
                    isPumpDebug = false;
                  }
                  // OK
                  SendOk(baseReceivedMessage);
                }
                else if (receivedArray[2] == "1")
                {
                  if (receivedArray[1] == DebugFastCMD)
                  {
                    isFastDebug = true;
                  }
                  else
                  {
                    isPumpDebug = true;
                  }
                  // OK
                  SendOk(baseReceivedMessage);
                }
                else
                {
                  // INVALID_PARAMETER
                  SendInvalidParameters(baseReceivedMessage);
                }
              }
            }
            else
            {
              // INVALID_COMMAND
              SendInvalidCommand(baseReceivedMessage);
            }
          }
        }
      }

      // Enable/Disable DEBUG_FAST Mode
      if (isFastDebug)
      {
        Serial.print("$<");
        Serial.print("DF?");
        Serial.print("PH:");
        Serial.print(PHValue, 3); // PH sensor Value
        Serial.print(",");
        Serial.print("TEMP:");
        Serial.print(TemperatureValue, 3); // Temperature sensor Value
        Serial.print(",");
        Serial.print("GS:");
        Serial.print(GravValue, 3); // Gravity sensor value
        Serial.println(">&");
      }

      // Enable/Disable DEBUG_FAST Mode
      if (isPumpDebug)
      {
        Serial.print("$<");
        Serial.print("DP?");
        Serial.print("1:");
        Serial.print(PUMP_1_Value); // PUMP 1
        Serial.print(",");
        Serial.print("2:");
        Serial.print(PUMP_2_Value); // PUMP 2
        Serial.print(",");
        Serial.print("3:");
        Serial.print(PUMP_3_Value); // PUMP 3
        Serial.print(",");
        Serial.print("4:");
        Serial.print(PUMP_4_Value); // PUMP 4
        Serial.println(">&");
      }

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
    PHValue = 3.5 * phValue;                          // convert the millivolt into pH value
  }
}