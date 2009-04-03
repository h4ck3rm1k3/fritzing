from django.utils.datastructures import SortedDict

AVAIL_SCRIPTS = [
    {'value':'resistor','label':'Resistor'},
    {'value':'mystery','label':'Mystery part'},
    {'value':'generic-male-header','label':'Generic male header'},
    {'value':'generic-female-header','label':'Generic female header'},
    {'value':'generic-female-header-rounded','label':'Generic female metal rounded header'},
]

SCRIPTS_DEFS = {
    'resistor' : SortedDict(),
    'mystery' : SortedDict(),
    'generic-male-header' : SortedDict(),
    'generic-female-header' : SortedDict(),
    'generic-female-header-rounded' : SortedDict(),
}

SCRIPTS_DEFS['mystery']['name'] = {
    'label'   : 'Part name',
    'type'    : 'string',
    'initial' : 'Generated Mystery Part'
}

_PINS_DICT_AUX = {
    'label': 'Number of pins',
    'type' : 'enum',
    'choices' : [
        (1,1),(2,2),(3,3),(4,4),(5,5),(6,6),(7,7),(8,8),(9,9),
        (10,10),(11,11),(12,12),(14,14),(16,16),(18,18),(20,20)
    ]    
}

SCRIPTS_DEFS['resistor']['resistance'] = {
    'label': 'Choose the resistance',
    'type' : 'regex',
    # just two significant digits
    'regex': '^\s*(([1-9]\d(0{0,6}|0{0,3}\s*[kK]?|\s*[kKmM]?)(\.0*)?)|(([1-9]\.\d0*)|(0\.[1-9]\d?0*))\s*[kKmM]?)\s*$',
    'error_messages' : {'invalid' : "You should provide a resistance with only two "+\
                        "significant digits, with one tenth decimal precision, for values "+\
                        "lower than 1 and a maximum of 8 whole digits."},
    'help_text' : "Provide a positive value with only two significant digits, with one "+\
                    "tenth decimal precision, for values lower than 1 and a maximum of 8 whole "+\
                    "digits. You can use 'k' and 'M' as units as well."+\
                    "<br/>For example: 10k, 0.2, 0.11, 0.5 M and 75000000 are all "+\
                    "valid values, but 0.01, 0.507, 113.10 k, 750000000 and 123000 are not"
}

SCRIPTS_DEFS['mystery']['pins'] = _PINS_DICT_AUX
SCRIPTS_DEFS['generic-male-header']['pins'] = _PINS_DICT_AUX
SCRIPTS_DEFS['generic-female-header']['pins'] = _PINS_DICT_AUX
SCRIPTS_DEFS['generic-female-header-rounded']['pins'] = _PINS_DICT_AUX
