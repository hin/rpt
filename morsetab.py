morsetab = {
    '0': '-----',
    '1': '.----',
    '2': '..---',
    '3': '...--',
    '4': '....-',
    '5': '.....',
    '6': '-....',
    '7': '--...',
    '8': '---..',
    '9': '----.',
    '/': '-..-.',
    'A': '.-',
    'B': '-...',
    'C': '-.-.',
    'D': '-..',
    'E': '.',
    'F': '--.-',
    'G': '--.',
    'H': '....',
    'I': '..',
    'J': '.---',
    'K': '-.-',
    'L': '.-..',
    'M': '--',
    'N': '-.',
    'O': '---',
    'P': '.--.',
    'Q': '--.-',
    'R': '.-.',
    'S': '...',
    'T': '-',
    'U': '..-',
    'V': '...-',
    'W': '.--',
    'X': '-..-',
    'Y': '-.--',
    'Z': '--..',
    ':': '---...',
    ';': '-.-.-.',
    '=': '-...-',
    '?': '--..--',
    '@': '.--.-.',
}

def morsechar(c):
    return '0b1'+morsetab.get(c, '')[::-1].replace('.', '0').replace('-', '1')

keys = [key for key in morsetab]
keyrange = [chr(c) for c in range(ord(min(keys)), 1+ord(max(keys)))]

print '#include <stdint.h>'
print '#define MORSETAB_START %d'%ord(min(keys))
print '#define MORSETAB_END %d'%ord(max(keys))
print 'uint8_t morsetab[] = {'

for key in keyrange:
    print '    %10s, /* %s %s */'%(morsechar(key), key, morsetab.get(key, 'unknown'))

print '};'
print 'uint8_t morsechar(char c) {'
print '    if ((c >= MORSETAB_START) && (c <= MORSETAB_END))'
print '        return morsetab[c-MORSETAB_START];'
print '    return 0b1;'
print '}'
