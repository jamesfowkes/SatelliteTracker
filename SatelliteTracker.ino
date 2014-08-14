/* Arduino Library Includes */
#include <Wire.h>

/* Stepper Motor Libraries */
#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"

/* Serial Comms Helper Library */
#include <ArduinoUtil.h>
#include <SerialMessaging.h>

/* Utility Libraries */
#include <util_angle.h>

/* Application Includes */
#include "MachineIf.h"

/* Defines and Typedefs */

#define VERSION_STRING "2"

#define AZ_MOTOR_HOME_PIN 0
#define AL_MOTOR_HOME_PIN 1

#define BASE_SPR 200

#define AZIMUTH_DIRECTION 1
#define ALTITUDE_DIRECTION -1

/* Private Function Prototypes */
static void fwdAzimuthMotor(void);
static void revAzimuthMotor(void);
static void fwdAltitudeMotor(void);
static void revAltitudeMotor(void);

static void SerialMessageCallback(String* message);

static void initMotors(void);
//static void homeMotors(void);
static void releaseMotors(void);

static void resetMotorSpeed(void);
static void resetMotorPosition(void);

//static void updateMotorHomeStates(void);

static int spr(void);
static int tenthDegreesToSteps(int angleTenthsDegrees);

/* Private Variables and Objects */

static Adafruit_MotorShield motorShield = Adafruit_MotorShield();

static Adafruit_StepperMotor *azimuthMotor = motorShield.getStepper(200, 1);
static Adafruit_StepperMotor *altitudeMotor = motorShield.getStepper(200, 2);

static AccelStepper azimuthStepper = AccelStepper(fwdAzimuthMotor, revAzimuthMotor);
static AccelStepper altitudeStepper = AccelStepper(fwdAltitudeMotor, revAltitudeMotor);

static SerialMessaging s_SerialMessaging(SerialMessageCallback);

//static bool azMotorHomed = false;
//static bool alMotorHomed = false;

static int s_steppingMode = MICROSTEP;

void setup()
{
  pinMode(AZ_MOTOR_HOME_PIN, INPUT);
  pinMode(AL_MOTOR_HOME_PIN, INPUT);

  motorShield.begin();
  releaseMotors();

  s_SerialMessaging.Begin(115200);

  s_SerialMessaging.Print("Tracker v");
  s_SerialMessaging.Print(VERSION_STRING);
  s_SerialMessaging.Println(" Ready");
  s_SerialMessaging.Println("Send commands in AZxxxxALyyyy format");
  s_SerialMessaging.Println("where xxxx and yyyy are tenths of degree");
  s_SerialMessaging.Println("positions for azimuth and altitude respectively");
  s_SerialMessaging.Println("(assumes unit is positioned correctly for 0 azimuth)");
}

bool running = false;

void loop()
{
  bool stillRunning = azimuthStepper.run();
  stillRunning |= altitudeStepper.run();
  
  if (!stillRunning && running)
  {
      Serial.println("MOVC");
	  running = false;
  }
  
  if (serialEventRun) {
    serialEventRun(); 
  }
}

static void SerialMessageCallback(String* message)
{
  /* The message should come in format "AZxxxxALxxxx"
   	where AZxxxx represents the azimuth to point to in tenths
   	of degrees (e.g. AZ2307 represents an azimuth of 230.7 degrees)
   	and ALxxxx represents the same for the altitude */
  if ((message->substring(0,2) == "AZ") && (message->substring(6,8) == "AL"))
  {
    int requested_azimuth = message->substring(2, 6).toInt();
    int requested_altitude = message->substring(8, 12).toInt();
	
    translateMoveRequest(&requested_azimuth, &requested_altitude);

    int azSteps = tenthDegreesToSteps(requested_azimuth);
    int alSteps = tenthDegreesToSteps(requested_altitude);

    s_SerialMessaging.Print("Moving to ");
    s_SerialMessaging.Print(azSteps);
    s_SerialMessaging.Print(",");
    s_SerialMessaging.Print(alSteps);
    s_SerialMessaging.Print(" (");   
    s_SerialMessaging.Print(requested_azimuth);
    s_SerialMessaging.Print(",");
    s_SerialMessaging.Print(requested_altitude);
    s_SerialMessaging.Println(")");
  
	running = true;
	
    azimuthStepper.moveTo(AZIMUTH_DIRECTION * azSteps);
    altitudeStepper.moveTo(ALTITUDE_DIRECTION * alSteps);
  }
  else if (message->equals("RELEASE"))
  {
    s_SerialMessaging.Println("Releasing...");
    releaseMotors();
  }
  else if (message->equals("ENGAGE"))
  {
    s_SerialMessaging.Println("RDY");
    resetMotorPosition();
    resetMotorSpeed();
  }
}

static void releaseMotors(void)
{
  azimuthMotor->release();
  altitudeMotor->release();
}

static void fwdAzimuthMotor(void)	{ 
  azimuthMotor->onestep(FORWARD, s_steppingMode); 
}
static void revAzimuthMotor(void)	{ 
  azimuthMotor->onestep(BACKWARD, s_steppingMode); 
}
static void fwdAltitudeMotor(void)	{ 
  altitudeMotor->onestep(FORWARD, s_steppingMode); 
}
static void revAltitudeMotor(void)	{ 
  altitudeMotor->onestep(BACKWARD, s_steppingMode); 
}

static void resetMotorPosition(void)
{
  azimuthStepper.setCurrentPosition(0);
  altitudeStepper.setCurrentPosition(0);
}

static void resetMotorSpeed(void)
{
  azimuthStepper.setMaxSpeed(1000.0f);
  altitudeStepper.setMaxSpeed(1000.0f);

  azimuthStepper.setSpeed(500.0f);
  altitudeStepper.setSpeed(500.0f);

  azimuthStepper.setAcceleration(500.0f);
  altitudeStepper.setAcceleration(500.0f);

}

/*static void homeMotors(void)
 {	
 updateMotorHomeStates();
 
 while (azimuthStepper.distanceToGo() || altitudeStepper.distanceToGo())
 {
 updateMotorHomeStates();
 
 if (!azMotorHomed) { 
 azimuthStepper.move(1); 
 }
 if (!alMotorHomed) { 
 altitudeStepper.move(1); 
 }
 
 azimuthStepper.run();
 altitudeStepper.run();
 }
 resetMotorPosition();
 resetMotorSpeed();
 }*/
/*
static void updateMotorHomeStates(void)
 {
 azMotorHomed = digitalRead(AZ_MOTOR_HOME_PIN);
 alMotorHomed = digitalRead(AL_MOTOR_HOME_PIN);
 }
 */

/* Arduino library defined functions */
void serialEvent()
{
  s_SerialMessaging.SerialEvent();
}

/* Helper functions */
static int tenthDegreesToSteps(int angleTenthsDegrees)
{

  int steps = ((((long)spr() * (long)angleTenthsDegrees)) + 1800) / 3600;
  return steps;
}

static int spr(void)
{
  if ((s_steppingMode == SINGLE) || (s_steppingMode == DOUBLE))
  {
    return BASE_SPR;
  }
  else if (s_steppingMode == INTERLEAVE)
  {
    return BASE_SPR * 2; 
  }
  else if (s_steppingMode == MICROSTEP)
  {
    return BASE_SPR * MICROSTEPS;
  }

  return BASE_SPR;
}







