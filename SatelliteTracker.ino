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

/* Private Function Prototypes */
static void fwdAzimuthMotor(void);
static void revAzimuthMotor(void);
static void fwdAltitudeMotor(void);
static void revAltitudeMotor(void);

static void SerialMessageCallback(String* message);

static void homeMotors(void);
static void updateMotorHomeStates(void);

static int degreesToSteps(int spr, int angleTenthsDegrees);

/* Private Variables and Objects */

static Adafruit_MotorShield motorShield = Adafruit_MotorShield();

static Adafruit_StepperMotor *azimuthMotor = motorShield.getStepper(200, 1);
static Adafruit_StepperMotor *altitudeMotor = motorShield.getStepper(200, 2);

static AccelStepper azimuthStepper = AccelStepper(fwdAzimuthMotor, revAzimuthMotor);
static AccelStepper altitudeStepper = AccelStepper(fwdAltitudeMotor, revAltitudeMotor);

static SerialMessaging s_SerialMessaging(SerialMessageCallback);

static bool azMotorHomed = false;
static bool alMotorHomed = false;

void setup()
{
	pinMode(AZ_MOTOR_HOME_PIN, INPUT);
	pinMode(AL_MOTOR_HOME_PIN, INPUT);
	
	homeMotors();
	s_SerialMessaging.Begin(115200);
	
	s_SerialMessaging.Println("Tracker v1 Ready");
	s_SerialMessaging.Println("Send commands in AZxxxxALyyyy format");
	s_SerialMessaging.Println("where xxxx and yyyy are tenths of degree");
	s_SerialMessaging.Println("positions for azimuth and altitude respectively");
	s_SerialMessaging.Println("(assumes unit is positioned correctly for 0 azimuth)");
}

void loop()
{
	azimuthStepper.run(); // Provided by AccelStepper. Required call to move motors
	altitudeStepper.run(); // Provided by AccelStepper. Required call to move motors
	if (serialEventRun) { serialEventRun(); }
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
		
		azimuthStepper.runToNewPosition(degreesToSteps(200, azimuth));
		altitudeStepper.runToNewPosition(degreesToSteps(200, altitude));
	}
}

static void fwdAzimuthMotor(void)	{ azimuthMotor->onestep(FORWARD, SINGLE); }
static void revAzimuthMotor(void)	{ azimuthMotor->onestep(BACKWARD, SINGLE); }
static void fwdAltitudeMotor(void)	{ altitudeMotor->onestep(FORWARD, SINGLE); }
static void revAltitudeMotor(void)	{ altitudeMotor->onestep(BACKWARD, SINGLE); }

static void homeMotors(void)
{
	azimuthStepper.setMaxSpeed(10.0f);
	altitudeStepper.setMaxSpeed(10.0f);
	
	updateMotorHomeStates();
		
	while (azimuthStepper.distanceToGo() || altitudeStepper.distanceToGo())
	{
		updateMotorHomeStates();
		
		if (!azMotorHomed) { azimuthStepper.move(1); }
		if (!alMotorHomed) { altitudeStepper.move(1); }
		
		azimuthStepper.run();
		altitudeStepper.run();
	}

        azimuthStepper.setCurrentPosition(0);
        altitudeStepper.setCurrentPosition(0);
}

static void updateMotorHomeStates(void)
{
	azMotorHomed = digitalRead(AZ_MOTOR_HOME_PIN);
	alMotorHomed = digitalRead(AZ_MOTOR_HOME_PIN);
}

/* Arduino library defined functions */
void serialEvent()
{
	s_SerialMessaging.SerialEvent();
}

/* Helper functions */
static int degreesToSteps(int spr, int angleTenthsDegrees)
{
	int steps = ((((long)spr * (long)angleTenthsDegrees)) + 1800) / 3600;
	return steps;
}
