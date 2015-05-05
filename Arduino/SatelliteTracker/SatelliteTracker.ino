/* Machine setup (should be first so that other headers can configure themselves )*/
#include "MachineIf.h"

/* Arduino Library Includes */
#include <Wire.h>
#include <TaskAction.h>

/* Stepper Motor Libraries */
#include <AccelStepper.h>
#if MACHINE_ID == 1
#include <Adafruit_MotorShield.h>
#endif

/* Serial Comms Helper Library */
#include <ArduinoUtil.h>
#include <SerialMessaging.h>

/* Utility Libraries */
#include "util_angle.h"

/* Application Includes */
#include "PositionTracker.h"
#include "DebugHandler.h"

/* Defines and Typedefs */

#define VERSION_STRING "4"

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

#if MACHINE_ID == 1
static int s_steppingMode = MICROSTEP;
#endif
static bool bStopRequest = false;

static AccelStepper * steppers[2];

static PositionTracker s_positionTracker[2] = {
    PositionTracker(10, 11, 200, "Azimuth"),
    PositionTracker(A2, A3, 200, "Elevation")
};

static TaskAction positionUpdateTask(positionUpdateTaskFn, 50, INFINITE_TICKS);

static long s_targetSpeedStepsPerS[2] = {0L, 0L};

static float s_maxSpeedsStepsPerS[2] = {200.0f, 500.0f};

static int s_debugState[2] = {0,0};

static DEBUG_HANDLER_STRUCT s_debugFunctions = 
{
    debugAzPosition,
    debugAlPosition,
    debugAzSpeed,
    debugAlSpeed
};

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

    s_SerialMessaging.Println("OFFLINE");

    steppers[AZ_MOTOR] = &azimuthStepper;
    steppers[AL_MOTOR] = &altitudeStepper;
    
    DebugInit(&s_debugFunctions);
}

void loop()
{
    positionUpdateTask.tick();
    
    calculateAndSetNewRunSpeed(AZ_MOTOR);
    calculateAndSetNewRunSpeed(AL_MOTOR);
    
    azimuthStepper.runSpeed();
    altitudeStepper.runSpeed();

    if (bStopRequest)
    {
        bStopRequest = false;
        s_SerialMessaging.Println("FREE");
        releaseMotors();
    }
    
    if (serialEventRun) {
        serialEventRun(); 
    }

    DebugTick();
    heartbeat();
}

static bool isTargetMessage(String * message)
{
    return ((message->substring(AZ_STRT-2, AZ_STRT) == "AZ") && (message->substring(AL_STRT-2, AL_STRT) == "AL"));
}

static long getNumeric(String *message, int start, int finish)
{
    char num[10];

    if (finish == -1) { finish = message->length(); }

    message->substring(start, finish).toCharArray(num, 10);
    return strtol(num, NULL, 10);
}

static void handleTarget(String * message)
{
    long requested_azimuth = getNumeric(message, AZ_STRT, AZ_END);
    long requested_altitude = getNumeric(message, AL_STRT, AL_END);

    translateMoveRequest(&requested_azimuth, &requested_altitude);

    s_SerialMessaging.Print("New target ");
    s_SerialMessaging.Print(requested_azimuth);
    s_SerialMessaging.Print(",");
    s_SerialMessaging.Print(requested_altitude);
    s_SerialMessaging.Println(".");   
    
    s_positionTracker[AZ_MOTOR].SetTarget(requested_azimuth);
    s_positionTracker[AZ_MOTOR].SetTarget(requested_altitude);
}

static void setCurrentMotorPosition(String* message, int motorID)
{
    if (motorID > AL_MOTOR) { return; }
    
    long positionInDegrees = getNumeric(message, 3, 8) / 100;

    s_SerialMessaging.Print(motorID == AZ_MOTOR ? "Setting new azimuth " : "Setting new altitude ");
    s_SerialMessaging.Print(positionInDegrees);
    
    s_positionTracker[motorID].SetPosition(positionInDegrees);
}

static void calculateAndSetNewRunSpeed(int motorID)
{
    long positionHDegrees = s_positionTracker[motorID].GetPosition();
    long targetHDegrees = s_positionTracker[motorID].GetPosition();
    long dPosition = positionHDegrees - targetHDegrees;
    long newSpeed = 0;
    
    if (dPosition > 18000) { dPosition -= 18000; }
    else if (dPosition < -18000) { dPosition += 36000; }    
    
    if (abs(dPosition) > 1000)
    {
        // Forget accurate tracking, just set maximum speed to match position as fast as possible
        s_debugState[motorID] = 0;
        steppers[motorID]->setSpeed(s_maxSpeedsStepsPerS[motorID]);
    }
    else
    {
        /*
        Within 10 degrees of actual, there are five possibilities:
        1: Behind position, speed lower than target
        2: Ahead of position, speed lower than target
        3: Behind position, speed higher than target
        4: Ahead of position, speed higher than target
        5: Position is exactly correct
        */
        bool tooSlow = s_targetSpeedStepsPerS[motorID] > steppers[motorID]->speed();
        
        if ((dPosition < 0) and (tooSlow))
        { 
            // Condition 1: set speed to target + 10%
            s_debugState[motorID] = 1;
            newSpeed = s_targetSpeedStepsPerS[motorID] / 10;
            newSpeed += s_targetSpeedStepsPerS[motorID];
        }
        else if ((dPosition > 0) and (tooSlow))
        { 
            // Condition 2: Too slow, but ahead of position, so keep this speed (do nothing)
            s_debugState[motorID] = 2;
            newSpeed = s_targetSpeedStepsPerS[motorID];
        }
        else if ((dPosition < 0) and (!tooSlow))
        { 
            // Condition 3: Too fast, but behind position, so keep this speed (do nothing)
            s_debugState[motorID] = 3;
            newSpeed = s_targetSpeedStepsPerS[motorID];
        }
        else if ((dPosition > 0) and (!tooSlow))
        { 
            // Condition 4: set speed to target - 10%
            s_debugState[motorID] = 4;
            newSpeed = -s_targetSpeedStepsPerS[motorID] / 10;
            newSpeed += s_targetSpeedStepsPerS[motorID];
        }
        else
        {
            newSpeed = s_targetSpeedStepsPerS[motorID];
        }
        steppers[motorID]->setSpeed(newSpeed);
    }
}

static void setTargetSpeed(String* message, int motorID)
{
    // Target speed in message is millionths of degrees per second
    s_targetSpeedStepsPerS[motorID] = getNumeric(message, 3, -1);
    
    // Assuming this represents hundreths of degrees per second, convert to steps per second
    s_targetSpeedStepsPerS[motorID] = hundrethDegreesToSteps(s_targetSpeedStepsPerS[motorID], motorID);
    
    // Then apply the final conversion to steps per second
    s_targetSpeedStepsPerS[motorID] += 5000;
    s_targetSpeedStepsPerS[motorID] /= 10000;
}

static void SerialMessageCallback(String* message)
{
    /* The message should come in format "AZxxxxALxxxx"
    where AZxxxx represents the azimuth to point to in hundreths
    of degrees (e.g. AZ23073 represents an azimuth of 230.73 degrees)
    and ALxxxx represents the same for the altitude */
    if (isTargetMessage(message))
    {
        handleTarget(message);
    }
    else if (message->equals("RELEASE"))
    {
        azimuthStepper.stop();
        altitudeStepper.stop();
        bStopRequest = true;    
    }
    else if (message->equals("ENGAGE"))
    {
        engageMotors();
        s_SerialMessaging.Println("ONLINE");
        resetMotorSpeed();
    }
    else if (message->startsWith("AZP"))
    {
        setCurrentMotorPosition(message, AZ_MOTOR);
    }
    else if (message->startsWith("ALP"))
    {
        setCurrentMotorPosition(message, AL_MOTOR);
    }
    else if (message->startsWith("ALS"))
    {
        setTargetSpeed(message, AL_MOTOR);
    }
    else if (message->startsWith("AZS"))
    {
        setTargetSpeed(message, AZ_MOTOR);
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
    else if (message->equals("AZDBGON"))
    {
        DebugSet(AZ_POSITION, true);
        DebugSet(AZ_SPEED, true);
    }
    else if (message->equals("ALDBGON"))
    {
        DebugSet(AL_POSITION, true);
        DebugSet(AL_SPEED, true);
    }
    else if (message->equals("AZDBGOFF"))
    {
        DebugSet(AZ_POSITION, false);
        DebugSet(AZ_SPEED, false);
    }
    else if (message->equals("ALDBGOFF"))
    {
        DebugSet(AL_POSITION, false);
        DebugSet(AL_SPEED, false);
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
    s_positionTracker[AZ_MOTOR].SetPosition(0);
    s_positionTracker[AL_MOTOR].SetPosition(-9000);
}

static void resetMotorSpeed(void)
{
    azimuthStepper.setMaxSpeed( s_maxSpeedsStepsPerS[AZ_MOTOR] );
    altitudeStepper.setMaxSpeed( s_maxSpeedsStepsPerS[AL_MOTOR] );

    azimuthStepper.setSpeed(s_targetSpeedStepsPerS[AZ_MOTOR]);
    altitudeStepper.setSpeed(s_targetSpeedStepsPerS[AL_MOTOR]);

    azimuthStepper.setAcceleration(50.0f);
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

static void positionUpdateTaskFn(void)
{
    s_positionTracker[AZ_MOTOR].Update();
    s_positionTracker[AL_MOTOR].Update();
}

static void debugSpeed(int motorID)
{
    Serial.println(motorID == AZ_MOTOR ?  "Azimuth speed:" : "Altitude speed:");
    Serial.print("\tTarget: "); Serial.println(s_targetSpeedStepsPerS[motorID]);
    Serial.print("\tActual: "); Serial.println(steppers[motorID]->speed());
    Serial.print("\tState: "); Serial.println(s_debugState[motorID]);
}

static void debugAzPosition(void) { s_positionTracker[AZ_MOTOR].Debug(); }
static void debugAlPosition(void) { s_positionTracker[AL_MOTOR].Debug(); }
static void debugAzSpeed(void){ debugSpeed(AZ_MOTOR); }
static void debugAlSpeed(void) { debugSpeed(AL_MOTOR); }