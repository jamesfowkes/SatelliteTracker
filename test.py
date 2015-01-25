import sys

for targetA in range(361):
    for sourceA in range(361):
        a = targetA - sourceA
        if a > 180:
            b = a - 360
        elif a < -180:
            b = a + 360
        else:
            b = a
        
        try:
            assert(b <= 180 and b >= -180)
        except:
            print("AssertionError with source = %d, target = %d, a = %d, b = %d" % (sourceA, targetA, a, b))
            sys.exit()
            
        if sourceA % 10 == 0 and targetA % 10 == 0 :
            print("S: %d, T: %d, A: %d, B: %d" % (sourceA, targetA, a, b))
        