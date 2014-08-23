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

#if MACHINE_ID == 2
#define GEAR_RATIO 26.85f
#endif

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

static void setupMachine(void);
static void heartbeat(void);

/* Private Variables and Objects */

#if MACHINE_ID == 1
static Adafruit_MotorShield motorShield = Adafruit_MotorShield();
static Adafruit_StepperMotor *azimuthMotor = motorShield.getStepper(200, 1);
static Adafruit_StepperMotor *altitudeMotor = motorShield.getStepper(200, 2);
#elif MACHINE_ID == 2
#define AZ_DIR_PIN 2
#define AZ_STEP_PIN 3
#define AZ_EN_PIN 4
#define AL_DIR_PIN 5
#define AL_STEP_PIN 6
#define AL_EN_PIN 7
#endif

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

  pinMode(13, OUTPUT);

  setupMachine();
  releaseMotors();

  s_SerialMessaging.Begin(115200);

  s_SerialMessaging.Print("Tracker v");
  s_SerialMessaging.Print(VERSION_STRING);
  s_SerialMessaging.Println(" Ready");
  s_SerialMessaging.Print("Built for" );
#if MACHINE_ID == 1
  s_SerialMessaging.Print(" prototype ");
#elif MACHINE_ID == 2
  s_SerialMessaging.Print(" massive ");
#endif
  s_SerialMessaging.Println("tracker.");
  s_SerialMessaging.Println("Send commands in AZxxxxALyyyy format");
  s_SerialMessaging.Println("where xxxx and yyyy are tenths of degree");
  s_SerialMessaging.Println("positions for azimuth and altitude respectively");
  s_SerialMessaging.Println("(assumes unit is positioned correctly for 0 azimuth)");
}

bool running = false;
bool test[] = {
  false, false};

void loop()
{
  bool stillRunning = test[0] || test[1];

  if (test[0])
  {
    (void)azimuthStepper.runSpeed();
  }
  else
  {
    stillRunning |= azimuthStepper.run();
  }

  if (test[1])
  {
    stillRunning |= altitudeStepper.runSpeed();
  }
  else
  {
    stillRunning |= altitudeStepper.run();
  }

  if (!stillRunning && running)
  {
    Serial.println("MOVC");
    running = false;
  }

  if (serialEventRun) {
    serialEventRun(); 
  }

  heartbeat();
}

static bool isMoveMessage(String * message)
{
  return ((message->substring(0,2) == "AZ") && (message->substring(7,9) == "AL"));
}

static long getAzimuth(String *message)
{
  char num[6];
  message->substring(2, 7).toCharArray(num, 6);
  return strtol(num, NULL, 10);
  //return message->substring(2, 7).toInt();
}

static long getAltitude(String *message)
{
  char num[6];
  message->substring(9, 14).toCharArray(num, 6);
  return strtol(num, NULL, 10);
  //return message->substring(9, 14).toInt();
}

static void SerialMessageCallback(String* message)
{
  /* The message should come in format "AZxxxxALxxxx"
   	where AZxxxx represents the azimuth to point to in tenths
   	of degrees (e.g. AZ2307 represents an azimuth of 230.7 degrees)
   	and ALxxxx represents the same for the altitude */
  if (isMoveMessage(message))
  {
    long requested_azimuth = getAzimuth(message);
    long requested_altitude = getAltitude(message);

    translateMoveRequest(&requested_azimuth, &requested_altitude);

    int azSteps = hundrethDegreesToSteps(requested_azimuth);
    int alSteps = hundrethDegreesToSteps(requested_altitude);

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
    s_SerialMessaging.Println("FREE");
    releaseMotors();
  }
  else if (message->equals("ENGAGE"))
  {
    engageMotors();
    s_SerialMessaging.Println("RDY");
    resetMotorPosition();
    resetMotorSpeed();
  }
  else if (message->equals("AZTEST"))
  {
    running = true;
    azimuthStepper.setSpeed(500);
    test[0] = true;
  }
  else if (message->equals("ALTEST"))
  {
    running = true;
    altitudeStepper.setSpeed(500);
    test[1] = true;
  }
  else if (message->startsWith("TS")
  {
    
  }
}

static void engageMotors(void)
{
#if MACHINE_ID == 2
  digitalWrite(AZ_EN_PIN, LOW);
  digitalWrite(AL_EN_PIN, LOW);
#endif
}

static void releaseMotors(void)
{
#if MACHINE_ID == 1
  azimuthMotor->release();
  altitudeMotor->release();
#elif MACHINE_ID == 2
  digitalWrite(AZ_EN_PIN, HIGH);
  digitalWrite(AL_EN_PIN, HIGH);
#endif
}

static void fwdAzimuthMotor(void)	{
#if MACHINE_ID == 1
  azimuthMotor->onestep(FORWARD, s_steppingMode); 
#elif MACHINE_ID == 2
  digitalWrite(AZ_DIR_PIN, AZIMUTH_DIRECTION == 1 ? HIGH : LOW);
  digitalWrite(AZ_STEP_PIN, HIGH);
  digitalWrite(AZ_STEP_PIN, LOW);
#endif
}
static void revAzimuthMotor(void)	{ 
#if MACHINE_ID == 1
  azimuthMotor->onestep(BACKWARD, s_steppingMode); 
#elif MACHINE_ID == 2
  digitalWrite(AZ_DIR_PIN, AZIMUTH_DIRECTION == 1 ? LOW : HIGH);
  digitalWrite(AZ_STEP_PIN, HIGH);
  digitalWrite(AZ_STEP_PIN, LOW);
#endif
}
static void fwdAltitudeMotor(void)	{ 
#if MACHINE_ID == 1
  altitudeMotor->onestep(FORWARD, s_steppingMode); 
#elif MACHINE_ID == 2
  digitalWrite(AL_DIR_PIN, ALTITUDE_DIRECTION == 1 ? HIGH : LOW);
  digitalWrite(AL_STEP_PIN, HIGH);
  digitalWrite(AL_STEP_PIN, LOW);
#endif
}
static void revAltitudeMotor(void)	{ 
#if MACHINE_ID == 1
  altitudeMotor->onestep(BACKWARD, s_steppingMode); 
#elif MACHINE_ID == 2
  digitalWrite(AL_DIR_PIN, ALTITUDE_DIRECTION == 1 ? LOW : HIGH);
  digitalWrite(AL_STEP_PIN, HIGH);
  digitalWrite(AL_STEP_PIN, LOW);
#endif
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

  azimuthStepper.setAcceleration(25.0f);
  altitudeStepper.setAcceleration(25.0f);
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
static int hundrethDegreesToSteps(long angleHundrethsDegrees)
{

  int steps = ((((long)spr() * angleHundrethsDegrees)) + 18000) / 36000;
  return steps;
}

static int spr(void)
{
#if MACHINE_ID == 1
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
#elif MACHINE_ID == 2
  return (int)((float)BASE_SPR * GEAR_RATIO);
#endif
}

static void setupMachine(void)
{
#if MACHINE_ID == 1
  motorShield.begin();
#else
  pinMode(AZ_DIR_PIN, OUTPUT);
  pinMode(AZ_STEP_PIN, OUTPUT);
  pinMode(AZ_EN_PIN, OUTPUT);
  pinMode(AL_DIR_PIN, OUTPUT);
  pinMode(AL_STEP_PIN, OUTPUT);
  pinMode(AL_EN_PIN, OUTPUT);
#endif
}

static void heartbeat(void)
{
  static int oldMillis = 0;
  static bool state = false;
  if (millis() - oldMillis > 500)
  {
    oldMillis = millis();
    digitalWrite(13, state = !state);
  } 
}



