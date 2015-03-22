import math

length = 1024

print '#include <stdint.h>'
print '#include <avr/pgmspace.h>'
print 'const int8_t sintable[]  = {'
for i in range(length):
    print '  %d,'%(127*math.sin(2*math.pi*i/length))
print '};'
