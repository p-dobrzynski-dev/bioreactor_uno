#include <Arduino.h>
#include <Servo.h>
#include <pump.h>

// Reading variables
float PHValue;
float TemperatureValue;
float GravValue;

// Create servo pumps objects
Servo DfRobotPump;

int DFRobotPumpPin = 3; // DFRobotPump Output (PUMP_0)

// which analog pin to connect
#define THERMISTORPIN A1

// Analog read samples
#define NUMSAMPLES 10

// Thermistor settings
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000

Pump pumpsArray[3] = {
    Pump(5, 7, 8),   // PUMP_1
    Pump(6, 9, 10),  // PUMP_2
    Pump(11, 12, 13) // PUMP_3
};

// Debug flags
bool isFastDebug = true;
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

const String SetPumpCMD = "SET_PUMP";

ERROR errorCode; // Error base object

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

  // Setting dfrobot pump
  DfRobotPump.attach(DFRobotPumpPin);
}

/*--------------------------------------------------*/
/*---------------------- Helpers -------------------*/
/*--------------------------------------------------*/

#pragma region terminal_helpers

void PrintInfo()
{
  Serial.println();
  Serial.println("/*---------------------- INFO ----------------------*/");
  Serial.println();
  Serial.println("?");
  Serial.println("CMD,DEBUG_FAST,{on/off}");
  Serial.println("CMD,DEBUG_PUMP,{on/off}");
  Serial.println("CMD,SET_PUMP,{n},{x}");
}

bool IsStringInt(String stringToCheck)
{
  for (byte i = 0; i < stringToCheck.length(); i++)
  {
    if (isDigit(stringToCheck.charAt(i)))
      return true;
  }
  return false;
}

void setPumpMotorSpeed(int index, int value)
{
  // Setting data for debug pump frame

  // Setting motor speed
  if (index == 0)
  {
    int pumpSpeed = map(value, -255, 255, 0, 180);
    DfRobotPump.write(pumpSpeed);
  }
  else
  {
    pumpsArray[index - 1].setPumpSpeed(value);
  }
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

double avergearray(int *arr, int number)
{
  int i;
  int max, min;
  double avg;
  long amount = 0;
  if (number <= 0)
  {
    return 0;
  }
  if (number < 5)
  { // less than 5, calculated directly statistics
    for (i = 0; i < number; i++)
    {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  }
  else
  {
    if (arr[0] < arr[1])
    {
      min = arr[0];
      max = arr[1];
    }
    else
    {
      min = arr[1];
      max = arr[0];
    }
    for (i = 2; i < number; i++)
    {
      if (arr[i] < min)
      {
        amount += min; // arr<min
        min = arr[i];
      }
      else
      {
        if (arr[i] > max)
        {
          amount += max; // arr>max
          max = arr[i];
        }
        else
        {
          amount += arr[i]; // min<=arr<=max
        }
      } // if
    }   // for
    avg = (double)amount / (number - 2);
  } // if
  return avg;
}

#define PhSensorPin A0 // pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00    // deviation compensate
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth 40    // times of collection
int pHArray[ArrayLenth]; // Store the average value of the sensor feedback
int pHArrayIndex = 0;

void ReadValuesFromSensors()
{
  int buf[10], temp;
  unsigned long int avgValue;

  unsigned short initialCounter;

  int samples[NUMSAMPLES];

  // Getting PH Values

  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue, voltage;
  if (millis() - samplingTime > samplingInterval)
  {
    pHArray[pHArrayIndex++] = analogRead(PhSensorPin);
    if (pHArrayIndex == ArrayLenth)
      pHArrayIndex = 0;
    voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
    pHValue = 3.5 * voltage + Offset;
    samplingTime = millis();
  }
  if (millis() - printTime > printInterval) // Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {
    PHValue = pHValue;
    GravValue = voltage;
  }

  // Getting temperature readings
  uint8_t i;
  float average;

  // take N samples in a row, with a slight delay
  for (i = 0; i < NUMSAMPLES; i++)
  {
    samples[i] = analogRead(THERMISTORPIN);
    delay(5);
  }

  // average all the samples out
  average = 0;
  for (i = 0; i < NUMSAMPLES; i++)
  {
    average += samples[i];
  }
  average /= NUMSAMPLES;

  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;

  float steinhart;
  steinhart = average / THERMISTORNOMINAL;          // (R/Ro)
  steinhart = log(steinhart);                       // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                        // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                      // Invert
  steinhart -= 273.15;                              // convert absolute temp to C

  TemperatureValue = steinhart;

  delay(5);
}

/*--------------------------------------------------*/
/*---------------------- Main loop ---------------------*/
/*--------------------------------------------------*/

void loop()
{
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
          if (receivedArray[3] != NULL)
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
        else if (receivedArray[1] == SetPumpCMD)
        {
          if (IsStringInt(receivedArray[2]))
          {
            int pumpIndex = receivedArray[2].toInt();

            // Validating pump index
            if (0 <= pumpIndex <= 3)
            {
              if (IsStringInt(receivedArray[3]))
              {
                int pumpValue = receivedArray[3].toInt();

                if (-255 <= pumpValue <= 255)
                {
                  setPumpMotorSpeed(pumpIndex, pumpValue);

                  // OK
                  SendOk(baseReceivedMessage);
                }
                else
                {
                  // INVALID_PARAMETER
                  SendInvalidParameters(baseReceivedMessage);
                }
              }
              else
              {
                // INVALID_PARAMETER
                SendInvalidParameters(baseReceivedMessage);
              }
            }
            else
            {
              // INVALID_PARAMETER
              SendInvalidParameters(baseReceivedMessage);
            }
          }
          else
          {
            // INVALID_PARAMETER
            SendInvalidParameters(baseReceivedMessage);
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
    Serial.print("0:");
    int mappedValue = map(DfRobotPump.read(), 0, 180, -255, 255);
    Serial.print(mappedValue); // PUMP 0
    Serial.print(",");
    Serial.print("1:");
    Serial.print(pumpsArray[0].GetCurrentSpeed()); // PUMP 1
    Serial.print(",");
    Serial.print("2:");
    Serial.print(pumpsArray[1].GetCurrentSpeed()); // PUMP 2
    Serial.print(",");
    Serial.print("3:");
    Serial.print(pumpsArray[2].GetCurrentSpeed()); // PUMP 3
    Serial.println(">&");
  }

  // Getting values from read sensors
  ReadValuesFromSensors();
}
