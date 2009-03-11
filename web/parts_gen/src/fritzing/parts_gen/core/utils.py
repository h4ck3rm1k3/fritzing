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

units = { 'k': 1000, 'M': 1000000 }

def stripe1(resistance):
    real_resistance = to_number(resistance)
    ch = str(real_resistance)[0]
    return colors[color_bands[int(ch)]]
    
def stripe2(resistance):
    real_resistance = to_number(resistance)
    digit_cnt = get_whole_digits_count(real_resistance);
    if(digit_cnt >1 ):
        ch = str(real_resistance)[1]
    else:
        ch = str(real_resistance)[2]
    return colors[color_bands[int(ch)]]
    
def stripe3(resistance):
    real_resistance = to_number(resistance)
    fst_num = int(str(real_resistance)[0])
    correction = 0
    if fst_num == 0:
        fst_num = int(str(real_resistance)[2])
        correction = 1
    digit_cnt = get_whole_digits_count(real_resistance);
    if(digit_cnt > 1 ):
        snd_num = int(str(real_resistance)[1])
    else: # there's a '.' in the middle
        try:
            snd_num = int(str(real_resistance)[2+correction])
        except:
            snd_num = 0
    multiplier = get_multiplier(fst_num, snd_num, real_resistance)
    #print fst_num
    #print snd_num
    #print multiplier
    return colors[color_bands[1]]

def to_number(resistance_str):
    last_char = resistance_str[-1]
    if last_char in units.keys() :
        return float(resistance_str[:-1])*units[last_char]
    else:
        assert last_char.isdigit()
        return float(resistance_str)
    
def get_multiplier(fst_num,snd_num,real_resistance):
    digit_cnt = get_whole_digits_count(real_resistance);
    correction = 0
    if int(str(real_resistance)[0]) == 0 and snd_num != 0:
        correction = 1
    return 10**(digit_cnt-2-correction)
    
def get_whole_digits_count(real_resistance):
    digit_cnt = 0
    resist_str = str(real_resistance)
    i = 1
    char = resist_str[0]
    while char != '.' and i <= len(resist_str):
        digit_cnt += 1
        i += 1
        char = resist_str[i-1]
    return digit_cnt