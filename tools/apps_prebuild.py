import math

symbols = {
    'Alarm': 'clock',
    'HeartRate': 'heartBeat',
    'Metronome': 'drum',
    'Motion': 'chartLine',
    'Music': 'music',
    'Navigation': 'map',
    'Paddle': 'paddle',
    'Paint': 'paintbrush',
    'Steps': 'shoe',
    'StopWatch': 'stopWatch',
    'Timer': 'hourGlass',
    'Twos': '2',
}

with open('apps.txt', 'r') as file:
    apps = file.readlines()

apps = [app.strip() for app in apps]

print('Apps:')
print(apps)

lines = []
for app in apps:
    if app in symbols:
        if app == 'Twos':
            symbol = '"2"'
        else:
            symbol = f'Symbols::{symbols[app]}'
        # 10 spaces for correct indentation
        # '{' and '}' separate because format strings interfere with them
        # Symbols::symbol, Apps::app in the middle
        # \n at the end for correct format
        lines.append(' '*10 + '{' + f'{symbol}, Apps::{app}'  + '},\n')

# \n at start
# 8 space for correct indentation of closing braces
lines = '\n' + ''.join(lines) + ' '*8

print(lines)

# 6 apps per screen, result rounded up
n_screens = math.ceil(len(apps) / 6)
print(f'Created {n_screens} screens.')

with open('tools/template.h', 'r') as template:
    content = template.read()

content = content.replace('@N_SCREENS@', str(n_screens))
content = content.replace('@APPS@', lines)

with open('src/displayapp/screens/ApplicationList.h', 'w') as out:
    out.write(content)

print('Done.')
