color_bands = ['black', 'brown', 'red', 'orange', 'yellow', 'green', 'blue', 'purple', 'grey', 'white']
colors = {
    'black': 'rgb(0, 0, 0)',
    'brown': 'rgb(138, 61, 6)',
    'red': 'rgb(196, 8, 8)',
    'orange': 'rgb(255, 77, 0)',
    'yellow': 'rgb(255, 213, 0)',
    'green': 'rgb(0, 163, 61)',
    'blue': 'rgb(0, 96, 182)',
    'purple': 'rgb(130, 16, 210)',
    'grey': 'rgb(140, 140, 140)',
    'white': 'rgb(255, 255, 255)'
}

def stripe1(resistance):
    ch = resistance[0]
    return colors[color_bands[int(ch)]]
    
def stripe2(resistance):
    ch = resistance[1]
    return colors[color_bands[int(ch)]]
    
def stripe3(resistance):
    ch = resistance[2:]
    return colors[color_bands[1]]