#include <Arduino.h>
#include "PositionTracker.h"

PositionTracker::PositionTracker(int p1, int p2, int hdegrees_per_tick, char * name)
{
    m_p1 = p1;
    m_p2 = p2;
    
    pinMode(m_p1, INPUT_PULLUP);
    pinMode(m_p2, INPUT_PULLUP);
    
    m_pinStates[0] = digitalRead(m_p1) == HIGH;
    m_pinStates[1] = digitalRead(m_p2) == HIGH;
    
    m_position_hdegrees = 0;
    m_target_hdegrees = 0;
    m_hdegrees_per_tick = hdegrees_per_tick;
    m_last_tick_position_hdegrees = 0;
    
    m_name = name;
    
    m_stateChangeCounter = 0;
}

PositionTracker::~PositionTracker()
{

}

void PositionTracker::Debug(void)
{
    Serial.print(m_name);
    Serial.println(":");
    Serial.print("\tPosition: ");Serial.println(m_position_hdegrees);
    Serial.print("\tTarget: ");Serial.println(m_target_hdegrees);
    Serial.print("\tState: ");Serial.print(m_pinStates[0]);
    Serial.print(", ");Serial.println(m_pinStates[1]);
}

void PositionTracker::SetPosition(int newPosition)
{
    m_position_hdegrees = newPosition;
}

int PositionTracker::GetPosition(void)
{
    return m_position_hdegrees;
}

void PositionTracker::SetTarget(int newTarget)
{
    m_target_hdegrees = newTarget;
}

int PositionTracker::GetTarget(void)
{
    return m_target_hdegrees;
}

void PositionTracker::Update()
{
    bool newPinStates[2];
    
    newPinStates[0] = digitalRead(m_p1) == HIGH;
    newPinStates[1] = digitalRead(m_p2) == HIGH;
    
    bool stateHasChanged = (newPinStates[0] != m_pinStates[0]) || (newPinStates[1] != m_pinStates[1]);
    
    if (stateHasChanged)
    {
        char encoder_state = 0;
        encoder_state += m_pinStates[0] ? 1 : 0;
        encoder_state += m_pinStates[1] ? 2 : 0;
        encoder_state += newPinStates[0] ? 4 : 0;
        encoder_state += newPinStates[1] ? 8 : 0;
        
        /* encoder_state now contains a pattern ABCD in lowest four bits where:
        A = previous A channel value
        B = previous B channel value
        C = new A channel value
        D = new B channel value
        */
        
        switch (encoder_state & 0x0F)
        {
        case 1:
        case 7:
        case 8:
        case 14:
            m_stateChangeCounter++;
            break;
        case 2:
        case 4:
        case 11:
        case 13:
            m_stateChangeCounter--;
            break;
        default:
            break;
        }
        
        if (abs(m_stateChangeCounter) == 10)
        {
            m_stateChangeCounter = 0;
            
            if (m_stateChangeCounter > 0)
            {
                m_last_tick_position_hdegrees += m_hdegrees_per_tick; // Forwards
            }
            else
            {
                m_last_tick_position_hdegrees -= m_hdegrees_per_tick; //Backwards
            }
            m_position_hdegrees = m_last_tick_position_hdegrees;
            m_pinStates[0] = newPinStates[0];
            m_pinStates[1] = newPinStates[1];
        }
    }
}
