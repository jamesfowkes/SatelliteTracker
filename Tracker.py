from HardwareInterface import HardwareInterface
from task import TaskHandler
from time import sleep

class Tracker:
    
    def __init__(self, tleProvider, tleParser, hw):
        self.tleParser = tleParser
        self.tleProvider = tleProvider
        self.task_handler = TaskHandler(self)
        self.task = self.task_handler.add_function(self.updateTask, 100, False)
        
        self.hw = hw
        
    def Run(self, start_az, start_al):
    
        while not self.hw.IsReady:
            self.hw.Update()
        
        self.hw.SetAzimuth(start_az)
        self.hw.SetAltitude(start_al)
        self.hw.Start()
    
        self.task.set_active(True)
        
        while(True):
            sleep(0.01)
            self.task_handler.tick()
            
    def updateTask(self):
        try:
            if self.tleProvider.RefreshTLE(self.tleParser.tle) == False:
                print("Warning: TLE failed to refresh")
                      
            self.tleParser.update()
            
            self.hw.SetAzSpeed(self.tleParser.azimuthSpeed())
            self.hw.SetAlSpeed(self.tleParser.altitudeSpeed())
            self.hw.SetTargetAzimuth(self.tleParser.getAzimuth())
            self.hw.SetTargetAltitude(self.tleParser.getAltitude())
            
            self.hw.Update()
                        
        except KeyboardInterrupt:
            self.hw.Stop()
            exit()
