#include <Arduino.h>
#include <TaskAction.h>

#include "DebugHandler.h"

static void azPositionHandlerDebug(void);
static void alPositionHandlerDebug(void);
static void azSpeedHandlerDebug(void);
static void alSpeedHandlerDebug(void);

static TaskAction debugTasks[] = {
    TaskAction(azPositionHandlerDebug, 1000, INFINITE_TICKS),
    TaskAction(alPositionHandlerDebug, 1000, INFINITE_TICKS),
    TaskAction(azSpeedHandlerDebug, 1000, INFINITE_TICKS),
    TaskAction(alSpeedHandlerDebug, 1000, INFINITE_TICKS)
};

static DEBUG_HANDLER_STRUCT * s_debug_handlers;

void DebugInit(DEBUG_HANDLER_STRUCT * debug_handlers)
{
    s_debug_handlers = debug_handlers;
    char i;
    for (i = 0; i < MAX_MODULES; i++)
    {
        debugTasks[i].Enable(false);
    }
}
 
void DebugTick(void)
{
    char i;
    for (i = 0; i < MAX_MODULES; i++)
    {
        debugTasks[i].tick();
    }
}
    
void DebugSet(int id, bool on)
{
    if (id < MAX_MODULES)
    {
        debugTasks[id].Enable(on);
    }
}

static void azPositionHandlerDebug(void)
{
    if (s_debug_handlers->azPositionHandlerDbgFn) {s_debug_handlers->azPositionHandlerDbgFn();}
}

static void alPositionHandlerDebug(void)
{
    if (s_debug_handlers->alPositionHandlerDbgFn) {s_debug_handlers->alPositionHandlerDbgFn();}
}


static void azSpeedHandlerDebug(void)
{
    if (s_debug_handlers->azSpeedHandlerDbgFn) {s_debug_handlers->azSpeedHandlerDbgFn();}
}

static void alSpeedHandlerDebug(void)
{
    if (s_debug_handlers->alSpeedHandlerDbgFn) {s_debug_handlers->alSpeedHandlerDbgFn();}
}