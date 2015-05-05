/*
 * controller.cpp
 *
 */

/*
 * Standard Library Includes
 */

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "math.h"

/*
 * Local Application Includes
 */

#include "controller.h"

/*
 * Defines and typedefs
 */

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#define SWITCH_TO_TRACKING_ERROR (0.05f)
#define SPACING_RADIANS (M_PI / 90.0f)

/*
 * Private Functions
 */

static bool anglesBridgeZero(float a, float b)
{
    if (a == b) { return false; }

    float halfRevolution = M_PI;

    float lowest = min(a, b);
    float highest = max(a, b);

    float diff = highest - lowest;

    bool crosses = true;

    if (diff > halfRevolution)
    {
        // If the absolute difference between high and low is > 180degrees,
        // then the angle can only cross zero if the lowest is less than 180degrees
        crosses &= lowest < halfRevolution;
    }
    else
    {
        // If the absolute difference between high and low is < 180degrees,
        // then the angle can only cross zero if the lowest is greater than 180degrees
        // and the highest is less than 360degrees
        crosses &= lowest > halfRevolution;
        crosses &= highest < halfRevolution;
    }
    return crosses;
}

Position::Position(float startPosition)
{
    this->set(startPosition);
}

void Position::set(float pos)
{
    this->m_p = Position::normalise(pos);
}

void Position::add(float toAdd)
{
    this->m_p += toAdd;
    this->m_p = Position::normalise(this->m_p);
}

float Position::get(void)
{
    return this->m_p;
}

PositionError::PositionError()
{
    this->m_magnitude = 0;
    this->m_leading = false;
    this->m_lagging = true;
}

void PositionError::update(float target, float actual)
{
    this->m_magnitude = fabs(target - actual);

    if (anglesBridgeZero(target, actual))
    {
        this->m_leading = actual > target;
    }
    else
    {
        this->m_leading = target > actual;
    }
    this->m_lagging = !this->m_leading;
}

float PositionError::getMagnitude(void)
{
    return m_magnitude;
}

bool PositionError::isLeading(void)
{
    return this->m_leading;
}

Controller::Controller(float maxSpeed) :
    m_estimatedPosition(0), m_targetRelativePosition(0), m_lastKnownPosition(0),
    m_nextKnownPosition(0), m_error()
{
    this->m_speed = 0;
    this->m_targetRelativeSpeed = 0;
    this->m_maxSpeed = maxSpeed;
}

float Controller::nextPosition(void)
{
    return this->m_lastKnownPosition.get() + SPACING_RADIANS;
}

float Controller::distanceToNext(void)
{
    return fabs(this->m_nextKnownPosition.get() - this->m_estimatedPosition.get());
}

void Controller::tick(float t, float targetPos, float targetSpeed)
{
    this->m_targetRelativePosition.set( targetPos );
    this->m_estimatedPosition.add( -this->m_speed * t ); // Speed is +ve going anti-clockwise
    this->m_error.update(this->m_targetRelativePosition.get(), this->m_estimatedPosition.get());

    this->m_targetRelativeSpeed = targetSpeed;
    this->setNewSpeed();
}

float Controller::fractionalError(void)
{
    return this->m_error.getMagnitude() / (M_PI*2);
}

bool Controller::useFastMode(void)
{
    return this->fractionalError() > SWITCH_TO_TRACKING_ERROR;
}

void Controller::setNewSpeed(void)
{
    if (this->useFastMode())
    {
        // Run at fastest possible speed to catch up
        if (this->m_error.isLeading())
        {
            this->m_speed = -this->m_maxSpeed; // Ahead by a long way - run in reverse
        }
        else
        {
            this->m_speed = +this->m_maxSpeed;
        }
    }
    else
    {
        if (this->m_error.isLeading())
        {
            // Reduce speed to let target catch up
            this->m_speed = this->m_targetRelativeSpeed * 0.8;
        }
        else
        {
            // Increase speed to catch up
            // Set catch-up speed based on magnitude of error
            float speed_diff = (this->m_maxSpeed - this->m_targetRelativeSpeed) * this->fractionalError() / SWITCH_TO_TRACKING_ERROR;
            this->m_speed = (this->m_targetRelativeSpeed + speed_diff);
        }
    }
}