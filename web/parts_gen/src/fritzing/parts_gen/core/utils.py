color_bands = ['black', 'brown', 'red', 'orange', 'yellow', 'green', 'blue', 'purple', 'grey', 'white']
multipliers = {
    1 : 'black',
    10 : 'brown',
    100 : 'red',
    1000 : 'orange',
    10000 : 'yellow',
    100000 : 'green',
    1000000 : 'blue',
    0.1 : 'gold',
    0.01 : 'silver'
}

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
    'white': 'rgb(255, 255, 255)',
    'gold' : 'rgb(217, 217, 25)',
    'silver' : 'rgb(230, 232, 250)'
}

units = { 'k': 1000, 'M': 1000000 }

def get_first_number(resistance):
    real_resistance = to_number(resistance)
    str_real_resistance = str(real_resistance)
    
    correction = 0
    fst_num = int(str_real_resistance[0])
    if fst_num == 0:
        fst_num = int(str_real_resistance[2])
        correction = 1

    return fst_num, correction

def stripe1(resistance):
    fst_num, _ = get_first_number(resistance)
    return colors[color_bands[fst_num]]


def get_second_number(resistance, correction):
    real_resistance = to_number(resistance)
    str_real_resistance = str(real_resistance)
    
    digit_cnt = get_whole_digits_count(real_resistance);
    if(digit_cnt > 1 ):
        snd_num = int(str_real_resistance[1])
    else: # there's a '.' in the middle
        try:
            snd_num = int(str_real_resistance[2+correction])
        except:
            snd_num = 0

    return snd_num

def stripe2(resistance):
    _, correction = get_first_number(resistance)
    snd_num = get_second_number(resistance, correction)
    return colors[color_bands[snd_num]]

    
def stripe3(resistance):
    real_resistance = to_number(resistance)
    fst_num, correction = get_first_number(resistance)
    snd_num = get_second_number(resistance, correction)
    multiplier = get_multiplier(fst_num, snd_num, real_resistance)
    print fst_num
    print snd_num
    print multiplier
    return colors[multipliers[multiplier]]


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
    if int(str(real_resistance)[0]) == 0:
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


def escape_to_file_name(a_str):
    return a_str.replace(" ","_").replace(".","_")

