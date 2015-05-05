/* Arduino Library Includes */
#include <Wire.h>

/* Stepper Motor Libraries */
#include <AccelStepper.h>

/* Serial Comms Helper Library */
#include <ArduinoUtil.h>
#include <SerialMessaging.h>

/* Utility Libraries */
#include <util_angle.h>

/* Defines and Typedefs */

#define VERSION_STRING "2"

#define BASE_SPR 200

#define GEAR_RATIO 26.85f
#define PLATFORM_RATIO (730.0f / 60.0f)
#define PLATFORM_RATIO_TWEAK (180.0f / 172.5f)

#define ALTITUDE_DIRECTION 1

/* Private Variables and Objects */
#define AZ_DIR_PIN 2
#define AZ_STEP_PIN 3
#define AZ_EN_PIN 4

#define AL_DIR_PIN 5
#define AL_STEP_PIN 6
#define AL_EN_PIN 7

static AccelStepper altitudeStepper = AccelStepper(fwdAltitudeMotor, revAltitudeMotor);

void setup()
{
  pinMode(13, OUTPUT);
  
  Serial.begin(115200);
  
  setupMachine();
  engageMotors();
  altitudeStepper.setMaxSpeed(200.0f);
  altitudeStepper.setSpeed(100.0f);
}

bool running = false;

void loop()
{
  fwdAltitudeMotor();
  delay(5);
  //altitudeStepper.runSpeed();
  //heartbeat();
}

static void engageMotors(void)
{
  digitalWrite(AL_EN_PIN, LOW);
}

static void releaseMotors(void)
{
  digitalWrite(AL_EN_PIN, HIGH);
}

static void fwdAltitudeMotor(void)	{ 
  digitalWrite(AL_DIR_PIN, ALTITUDE_DIRECTION == 1 ? HIGH : LOW);
  digitalWrite(AL_STEP_PIN, HIGH);
  digitalWrite(AL_STEP_PIN, LOW);
}
static void revAltitudeMotor(void)	{ 
  digitalWrite(AL_DIR_PIN, ALTITUDE_DIRECTION == 1 ? LOW : HIGH);
  digitalWrite(AL_STEP_PIN, HIGH);
  digitalWrite(AL_STEP_PIN, LOW);
}

/* Helper functions */
static void setupMachine(void)
{
  pinMode(AL_DIR_PIN, OUTPUT);
  pinMode(AL_STEP_PIN, OUTPUT);
  pinMode(AL_EN_PIN, OUTPUT);

  pinMode(AZ_DIR_PIN, OUTPUT);
  pinMode(AZ_STEP_PIN, OUTPUT);
  pinMode(AZ_EN_PIN, OUTPUT);

  digitalWrite(AZ_EN_PIN, HIGH);
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

