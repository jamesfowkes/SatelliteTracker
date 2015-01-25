class PositionTracker
{
    public:
        PositionTracker(int p1, int p2, int hdegrees_per_tick, char * name);
        ~PositionTracker();
        
        void SetPosition(int newPosition);
        int GetPosition(void);
        
        void SetTarget(int newTarget);
        int GetTarget(void);

        void Update();
        
        void Debug();
        
    private:
    
        int m_p1;
        int m_p2;
        
        bool m_pinStates[2];
        
        long m_position_hdegrees;
        
        long m_target_hdegrees;
        
        long m_hdegrees_per_tick;
        
        long m_last_tick_position_hdegrees;
        
        int m_stateChangeCounter;
        
        char * m_name;
};
