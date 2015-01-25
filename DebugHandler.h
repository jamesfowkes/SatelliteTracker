enum modules
{
    AZ_POSITION,
    AL_POSITION,
    AZ_SPEED,
    AL_SPEED,
    MAX_MODULES
};

typedef void (*DEBUG_HANDLER_FN)(void);

struct debug_handler_struct
{
    DEBUG_HANDLER_FN azPositionHandlerDbgFn;
    DEBUG_HANDLER_FN alPositionHandlerDbgFn;
    DEBUG_HANDLER_FN azSpeedHandlerDbgFn;
    DEBUG_HANDLER_FN alSpeedHandlerDbgFn;
};
typedef struct debug_handler_struct DEBUG_HANDLER_STRUCT;

void DebugInit(DEBUG_HANDLER_STRUCT * debug_handlers); 
void DebugTick(void);
void DebugSet(int id, bool on);
