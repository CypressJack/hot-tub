#include <chrono>

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 5

#define PUMP 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

bool heater = false;
bool jets = false;
bool jetsRequested = false;
bool safe;
int target = 102;
int maxTemp = 109;
int maxHeatingMinutes = 120;
int dangerHeatingTime = 320;
int maxJetsMinutes = 25;
int schedulerInterval = 10;
int deadband = 2;

std::chrono::system_clock::time_point heatStartingTime;
std::chrono::system_clock::time_point jetsStartingTime;
std::chrono::system_clock::time_point lastSchedulerCheck;

void checkCurrentTemp()
{
  sensors.requestTemperatures();
  float currentTemp = sensors.getTempFByIndex(0);
  if (currentTemp > maxTemp)
  {
    safe = false;
    heater = false;
    jets = false;
  }
  if (currentTemp > (target + deadband) && heater == true)
  {
    heater = false;
    Serial.println("Heater has now turned off");
  }
  Serial.print("Safety Status: ");
  Serial.print(safe);
  Serial.print("\n");
  if ((currentTemp < (target - deadband) && heater != true) && safe == true)
  {
    heater = true;
    heatStartingTime = std::chrono::system_clock::now();
    Serial.println("Heater has now turned on");
  }
  Serial.print("Heater = ");
  Serial.print(heater);
  Serial.print("\n");
  Serial.print("Current Temp = ");
  Serial.print(currentTemp);
  Serial.print("\n");
}

void turnOnJets()
{
  if (safe == true)
  {
    jetsStartingTime = std::chrono::system_clock::now();
    jets = true;
    Serial.println("Jets has now turned on");
  }
}

void turnOffJets()
{
  jets = false;
  jetsRequested = false;
  Serial.println("Jets has now turned off");
}

// periodically check how long the heater has been running
void checkJetsTime()
{
  if (jets == true)
  {
    auto now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = now - jetsStartingTime;
    if ((elapsed.count() / 60) > maxJetsMinutes)
    {
      jets = false;
    }
    Serial.print("Current elapsed jets time: ");
    Serial.print(elapsed.count() / 60);
    Serial.print(" Minutes");
    Serial.print("\n");
  }
}

void manageJetsRequests()
{
  if (jetsRequested == true && jets == false)
  {
    turnOnJets();
  }
  if (jetsRequested == false && jets == true)
  {
    turnOffJets();
  }
  Serial.print("Jets status: ");
  Serial.print(jetsRequested);
  Serial.print("\n");
}

// periodically check how long the heater has been running
void checkHeatingTime()
{
  if (heater == true)
  {
    auto now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = now - heatStartingTime;
    Serial.print("Elapsed heating time: ");
    Serial.print(elapsed.count());
    Serial.print("\n");
    if ((elapsed.count() / 60) > maxHeatingMinutes)
    {
      heater = false;
    }
  }
}

// Prevent the heat from coming on if in runaway condition
// void killSwitch()
// {
//   if (heater == true)
//   {
//     auto now = std::chrono::system_clock::now();
//     std::chrono::duration<double> elapsed = now - heatStartingTime;
//     if ((elapsed.count() / 60) > dangerHeatingTime)
//     {
//       safe = false;
//       heater = false;
//       jets = false;
//     }
//   }
// }

void handlePinWrite() {
  if (heater == true) {
    digitalWrite(PUMP, LOW);
    return;
  }
  digitalWrite(PUMP, HIGH);
}

void scheduler()
{
  auto now = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed = now - lastSchedulerCheck;
  if ((elapsed.count()) > schedulerInterval)
  {
    Serial.print("\n");
    Serial.print("\n");
    Serial.print("\n");
    Serial.print("\n");
    Serial.print("\n");
    Serial.println("-------------------");
    manageJetsRequests();
    checkCurrentTemp();
    checkHeatingTime();
    checkJetsTime();
    // killSwitch();
    handlePinWrite();
    Serial.println("-------------------");
    lastSchedulerCheck = std::chrono::system_clock::now();
  }
}

void setup()
{
  Serial.begin(115200);
  lastSchedulerCheck = std::chrono::system_clock::now();
  safe = true;
  pinMode(PUMP, OUTPUT);
  digitalWrite(PUMP, HIGH);
}

void loop()
{
  scheduler();
  delay(1000);
}