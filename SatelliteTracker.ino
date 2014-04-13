/* Arduino Library Includes */
#include <Wire.h>

/* Stepper Motor Libraries */
#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"

/* Serial Comms Helper Library */
#include <SerialMessaging.h>

/* Defines and Typedefs */

#define AZ_MOTOR_HOME_PIN 0
#define AL_MOTOR_HOME_PIN 1

#define BASE_SPR 200

/* Private Function Prototypes */
static void fwdAzimuthMotor(void);
static void revAzimuthMotor(void);
static void fwdAltitudeMotor(void);
static void revAltitudeMotor(void);

static void SerialMessageCallback(String* message);

static void initMotors(void);
static void homeMotors(void);
static void resetMotorSpeed(void);
static void resetMotorPosition(void);

static void updateMotorHomeStates(void);

static int spr(void);
static int degreesToSteps(int angleTenthsDegrees);

/* Private Variables and Objects */

static Adafruit_MotorShield motorShield = Adafruit_MotorShield();

static Adafruit_StepperMotor *azimuthMotor = motorShield.getStepper(200, 1);
static Adafruit_StepperMotor *altitudeMotor = motorShield.getStepper(200, 2);

static AccelStepper azimuthStepper = AccelStepper(fwdAzimuthMotor, revAzimuthMotor);
static AccelStepper altitudeStepper = AccelStepper(fwdAltitudeMotor, revAltitudeMotor);

static SerialMessaging s_SerialMessaging(SerialMessageCallback);

static bool azMotorHomed = false;
static bool alMotorHomed = false;

static int s_steppingMode = INTERLEAVE;

void setup()
{
  pinMode(AZ_MOTOR_HOME_PIN, INPUT);
  pinMode(AL_MOTOR_HOME_PIN, INPUT);

  initMotors();
  resetMotorPosition();
  resetMotorSpeed();
  s_SerialMessaging.Begin(115200);

  s_SerialMessaging.Println("Tracker v1 Ready");
  s_SerialMessaging.Println("Send commands in AZxxxxALyyyy format");
  s_SerialMessaging.Println("where xxxx and yyyy are tenths of degree");
  s_SerialMessaging.Println("positions for azimuth and altitude respectively");
  s_SerialMessaging.Println("(assumes unit is positioned correctly for 0 azimuth)");
}

void loop()
{
  azimuthStepper.run();
  altitudeStepper.run();
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
    int azimuth = message->substring(2, 6).toInt();
    int altitude = message->substring(8, 12).toInt();

    int azSteps = degreesToSteps(azimuth);
    int alSteps = degreesToSteps(altitude);

    s_SerialMessaging.Print("Moving to ");
    s_SerialMessaging.Print(azSteps);
    s_SerialMessaging.Print(",");
    s_SerialMessaging.Println(alSteps);

    azimuthStepper.moveTo(azSteps);
    altitudeStepper.moveTo(alSteps);
  }
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

static void initMotors(void)
{
  motorShield.begin();
  resetMotorSpeed();
}

static void resetMotorPosition(void)
{
  azimuthStepper.setCurrentPosition(0);
  altitudeStepper.setCurrentPosition(0);
}

static void resetMotorSpeed(void)
{
  azimuthStepper.setMaxSpeed(100.0f);
  altitudeStepper.setMaxSpeed(100.0f);

  azimuthStepper.setSpeed(50.0f);
  altitudeStepper.setSpeed(50.0f);

  azimuthStepper.setAcceleration(100.0f);
  altitudeStepper.setAcceleration(100.0f);

}

static void homeMotors(void)
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
}

static void updateMotorHomeStates(void)
{
  azMotorHomed = digitalRead(AZ_MOTOR_HOME_PIN);
  alMotorHomed = digitalRead(AL_MOTOR_HOME_PIN);
}

/* Arduino library defined functions */
void serialEvent()
{
  s_SerialMessaging.SerialEvent();
}

/* Helper functions */
static int degreesToSteps(int angleTenthsDegrees)
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
    return BASE_SPR * 16;
  }
  
  return BASE_SPR;
}

