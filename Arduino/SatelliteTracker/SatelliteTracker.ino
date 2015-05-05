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

#define BASE_SPR 200

#if MACHINE_ID == 2
#define GEAR_RATIO 26.85f
#define PLATFORM_RATIO (730.0f / 60.0f)
#define PLATFORM_RATIO_TWEAK (180.0f / 172.5f)
#endif

#define AZIMUTH_DIRECTION -1
#define ALTITUDE_DIRECTION -1

#define AZ_MOTOR 0
#define AL_MOTOR 1

#define CHARS_PER_DEGREE_STRING (5)
#define AZ_STRT (2)
#define AZ_END (AZ_STRT + CHARS_PER_DEGREE_STRING)
#define AL_STRT (AZ_END + (2))
#define AL_END (AL_STRT + CHARS_PER_DEGREE_STRING)

/* Private Function Prototypes */
static void fwdAzimuthMotor(void);
static void revAzimuthMotor(void);
static void fwdAltitudeMotor(void);
static void revAltitudeMotor(void);

static void SerialMessageCallback(String* message);

static void initMotors(void);
static void releaseMotors(void);

static void resetMotorSpeed(void);
static void resetMotorPosition(void);

static long spr(int motorID);
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

static int s_steppingMode = MICROSTEP;
static bool bStopRequest = false;

void setup()
{
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
  s_SerialMessaging.Print("SPR(AZ) = ");
  s_SerialMessaging.Print(spr(AZ_MOTOR));
  s_SerialMessaging.Print(", SPR(AL) = ");
  s_SerialMessaging.Println(spr(AL_MOTOR));
}

bool running = false;
bool test[] = {
  false, false};

float runSpeedStepsPerSecond = 0.0f;

void loop()
{
  bool stillRunning = test[AZ_MOTOR] || test[AL_MOTOR];

  if (test[AZ_MOTOR])
  {
    (void)azimuthStepper.runSpeed();
  }
  else
  {
    stillRunning |= azimuthStepper.run();
  }

  if (test[AL_MOTOR])
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
    if (bStopRequest)
    {
      bStopRequest = false;
      s_SerialMessaging.Println("FREE");
      releaseMotors();
    }
  }

  if (serialEventRun) {
    serialEventRun(); 
  }

  //heartbeat();
}

static bool isMoveMessage(String * message)
{
  return ((message->substring(AZ_STRT-2, AZ_STRT) == "AZ") && (message->substring(AL_STRT-2, AL_STRT) == "AL"));
}

static long getNumeric(String *message, int start, int finish)
{
  char num[10];
  message->substring(start, finish).toCharArray(num, 10);
  return strtol(num, NULL, 10);
}

static void SerialMessageCallback(String* message)
{
  /* The message should come in format "AZxxxxALxxxx"
   	where AZxxxx represents the azimuth to point to in hundreths
   	of degrees (e.g. AZ23073 represents an azimuth of 230.73 degrees)
   	and ALxxxx represents the same for the altitude */
  if (isMoveMessage(message))
  {
    long requested_azimuth = getNumeric(message, AZ_STRT, AZ_END);
    long requested_altitude = getNumeric(message, AL_STRT, AL_END);

    translateMoveRequest(&requested_azimuth, &requested_altitude);

    long azSteps = hundrethDegreesToSteps(requested_azimuth, AZ_MOTOR);
    long alSteps = hundrethDegreesToSteps(requested_altitude, AL_MOTOR);

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
    azimuthStepper.moveTo(azSteps);
    altitudeStepper.moveTo(alSteps);
  }
  else if (message->equals("RELEASE"))
  {
    azimuthStepper.stop();
    altitudeStepper.stop();
    running = true;
    bStopRequest = true;    
  }
  else if (message->equals("ENGAGE"))
  {
    engageMotors();
    s_SerialMessaging.Println("RDY");
    test[AZ_MOTOR] = false;
    test[AL_MOTOR] = false;
    resetMotorSpeed();
  }
  else if (message->equals("AZTEST"))
  {
    running = true;
    azimuthStepper.setSpeed(500);
    test[AZ_MOTOR] = true;
  }
  else if (message->equals("ALTEST"))
  {
    running = true;
    altitudeStepper.setSpeed(500);
    test[AL_MOTOR] = true;
  }
  else if (message->startsWith("TS"))
  {
    long newSpeed = getNumeric(message, 2, 6);

    if (newSpeed)
    {
      s_SerialMessaging.Print("Setting new speed "); 
      s_SerialMessaging.Print(newSpeed);
      s_SerialMessaging.Println(" steps per second.");
      runSpeedStepsPerSecond = (float)newSpeed;
      azimuthStepper.setSpeed(runSpeedStepsPerSecond);
      altitudeStepper.setSpeed(runSpeedStepsPerSecond);
    }
  }
  else if (message->startsWith("AZP"))
  {
    long positionInHundrethsDegrees = getNumeric(message, 3, 8);

    long positionInSteps = hundrethDegreesToSteps(positionInHundrethsDegrees, AZ_MOTOR);

    s_SerialMessaging.Print("Setting new azimuth ");
    s_SerialMessaging.Print(positionInHundrethsDegrees);
    s_SerialMessaging.Print(" = ");
    s_SerialMessaging.Print(positionInSteps);
    s_SerialMessaging.Println(" steps");
    azimuthStepper.setCurrentPosition(positionInSteps);
  }
  else if (message->startsWith("ALP"))
  {
    long positionInHundrethsDegrees = 0;
    if(message->startsWith("ALP-"))
    {
      positionInHundrethsDegrees = getNumeric(message, 3, 9);
    }
    else
    {
      positionInHundrethsDegrees = getNumeric(message, 3, 8);
    }
    
    long positionInSteps = hundrethDegreesToSteps(positionInHundrethsDegrees, AL_MOTOR);

    s_SerialMessaging.Print("Setting new altitude ");
    s_SerialMessaging.Print(positionInHundrethsDegrees);
    s_SerialMessaging.Print(" = ");
    s_SerialMessaging.Print(positionInSteps);
    s_SerialMessaging.Println(" steps");
    altitudeStepper.setCurrentPosition(positionInSteps);
  }
  else if (message->equals("RSTP"))
  {
    resetMotorPosition();
  }
  else if (message->equals("?"))
  {
    s_SerialMessaging.Print("AZ = ");
    s_SerialMessaging.Print(azimuthStepper.currentPosition());
    s_SerialMessaging.Print(", AL = ");
    s_SerialMessaging.Println(altitudeStepper.currentPosition());
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
  altitudeStepper.setCurrentPosition( hundrethDegreesToSteps(-90L * 100L, AL_MOTOR) );
}

static void resetMotorSpeed(void)
{
  azimuthStepper.setMaxSpeed(500.0f);
  altitudeStepper.setMaxSpeed(200.0f);

  azimuthStepper.setSpeed(runSpeedStepsPerSecond);
  altitudeStepper.setSpeed(runSpeedStepsPerSecond);

  azimuthStepper.setAcceleration(5.0f);
  altitudeStepper.setAcceleration(50.0f);
}

/* Arduino library defined functions */
void serialEvent()
{
  s_SerialMessaging.SerialEvent();
}

/* Helper functions */
static long hundrethDegreesToSteps(long angleHundrethsDegrees, int motorID)
{
  uint64_t a = angleHundrethsDegrees >= 0 ? 18000LL : -18000LL;
  long steps = (long)(((((int64_t)spr(motorID) * (int64_t)angleHundrethsDegrees)) + angleHundrethsDegrees) / 36000LL);
  return steps;
}

static long spr(int motorID)
{
#if MACHINE_ID == 1
  (void)motorID;
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
  if (motorID == AZ_MOTOR)
  {
    return (long)(float)((float)BASE_SPR * GEAR_RATIO * PLATFORM_RATIO * PLATFORM_RATIO_TWEAK);
  }
  else
  {
    return (long)(float)((float)BASE_SPR * GEAR_RATIO);
  }
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
  resetMotorPosition();
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


