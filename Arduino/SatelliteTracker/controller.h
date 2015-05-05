#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

class Position
{
    public:
        Position(float startPosition);
        void set(float position);
        void add(float toAdd);
        float get(void);
        static float normalise(float pos)
        {
            while(pos > 2*M_PI) {pos -= M_PI*2;}
            return pos;
        }

    private:
        float m_p;
};

class PositionError
{
    public:
        PositionError();
        void update(float target, float actual);
        float getMagnitude(void);
        bool isLeading(void);

    private:
        float m_magnitude;
        bool m_leading;
        bool m_lagging;
};

class Controller
{
    public:
        Controller(float maxSpeed);
        void tick(float t, float target, float targetSpeed);

    private:
        float nextPosition(void);
        float distanceToNext(void);
        float fractionalError(void);
        bool useFastMode(void);
        void setNewSpeed(void);

        float m_speed;
        Position m_estimatedPosition;
        Position m_targetRelativePosition;
        Position m_lastKnownPosition;
        Position m_nextKnownPosition;
        float m_targetRelativeSpeed;
        PositionError m_error;
        float m_maxSpeed;
};

#endif

