from fritzing.parts_gen.core import partomatic
import os
from django.utils.datastructures import SortedDict

AVAIL_SCRIPTS = [
    {'value':'resistor','label':'Create a resistor'},
    {'value':'mystery','label':'Create a mystery part'}
]

_SCRIPTS_DEF = {
    'resistor' : SortedDict(),
    'mystery' : SortedDict(),
}

_SCRIPTS_DEF['mystery']['title'] = {
    'label'   : 'Part title',
    'type'    : 'string',
    'initial' : 'Generated Mystery Part'
}

_SCRIPTS_DEF['mystery']['pins'] = {
    'label': 'Amount of pins',
    'type' : 'enum',
    'choices' : [
        (1,1),(2,2),(3,3),(4,4),(5,5),(6,6),(7,7),(8,8),(9,9),
        (10,10),(11,11),(12,12),(14,14),(16,16),(18,18),(20,20)
    ]    
}

_SCRIPTS_DEF['resistor']['title'] =  {
    'label'   : 'Part title',
    'type'    : 'string',
    'initial' : 'Generated Resistor'
}
_SCRIPTS_DEF['resistor']['resistance'] = {
    'label': 'Choose the resistance',
    'type' : 'regex',
    # just two significant digits
    'regex': '^(([1..9]\d0*(\.0*)?)|([1..9]\.\d0*)|(0\.[1..9]\d?0*)) *[kM]?$',
    'error_messages' : {'invalid' : "You should provide a resistance with only two significant digits. You can use 'k' and 'M' as units as well."}
}

_GEN_FILES_FOLDER_PREFIX = os.path.join(os.path.dirname(__file__),"core/output")

def get_params_def(script_id):
    obj = _SCRIPTS_DEF[script_id] 
    return obj


def gen_files(script_id, config):
    return partomatic.web_generate(script_id, config, _GEN_FILES_FOLDER_PREFIX)
