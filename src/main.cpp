#include <chrono>

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 5

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

class Timer
{
public:
  bool timing = false;
  std::chrono::steady_clock::time_point startingTime;

  bool isTiming()
  {
    if (this->timing == true)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  void start()
  {
    if (this->timing == false)
    {
      this->startingTime = std::chrono::steady_clock::now();
      this->timing = true;
    }
  }
  void end()
  {
    if (this->timing == true)
    {
      this->startingTime = std::chrono::steady_clock::now();
      this->timing = false;
    }
  }
  auto elapsed()
  {
    if (this->timing == true)
    {
      auto now = std::chrono::steady_clock::now();
      std::chrono::duration<double> elapsed_seconds = now - startingTime;
      return elapsed_seconds.count();
    }
  }
};

class Controller
{
public:
  bool heater = false;
  bool jets = false;
  int pump = 16;
  int target = 100;
  int maxTemp = 103;
  int maxHeatingMinutes = 240;
  int maxJetsMinutes = 25;
  int deadband = 3;

  Timer heatElapsed;
  Timer jetsElapsed;

  // Tempuratures
  float currentTemp()
  {
    return sensors.getTempF(0);
  }
  void setTarget(int setPoint)
  {
    this->target = setPoint;
  }

  // Heater
  void heatOn()
  {
    if (this->currentTemp() < this->maxTemp)
    {
      this->heater = true;
      this->heatElapsed.start();
    }
  }
  void heatOff()
  {
    this->heater = false;
    this->heatElapsed.end();
  }

  // Jets
  void jetsOn()
  {
    this->jetsElapsed.start();
    this->jets = true;
  }

  void jetsOff()
  {
    this->jetsElapsed.end();
    this->jets = false;
  }
  // Safety Shutdown
  void safetyCheck()
  {
    if (this->currentTemp() > this->maxTemp)
    {
      this->jetsOff();
      this->heatOff();
    }
    if (this->heatElapsed.elapsed() > this->maxHeatingMinutes)
    {
      this->jetsOff();
      this->heatOff();
    }
    if (this->jetsElapsed.elapsed() > this->maxJetsMinutes)
    {
      this->jetsOff();
    }
  }
};

Controller hottub;

void setup()
{
  Serial.begin(9600);
  sensors.begin();
  hottub.setup();
}

void loop()
{
  // put your main code here, to run repeatedly:
}