from fritzing.parts_gen.core import partomatic
import os

AVAIL_SCRIPTS = [
    {'value':'resistor','label':'Create a resistor'}
]

_SCRIPTS_DEF = {
#    'resistor' : {
#        'resistance' : {
#            'label': 'Choose the resistance',
#            'type' : 'int',
#            'min_value' : 1,
#            'max_value' : 5
#        },
#        'unit' : {
#            'label': 'Choose the unit',
#            'type' : 'enum',
#            'options' : [('kv','kl'),('mv','ml')]
#        },
#        'regex' : {
#            'type' : 'regex',
#            'regex' : 'abcdef'
#        },
#        'float' : {
#            'type' : 'float'
#        }
#    }

    'resistor' : {
        'resistance' : {
            'label': 'Choose the resistance',
            'type' : 'regex',
            # just two significant digits
            'regex': '^(([1..9]\d0*(\.0*)?)|([1..9]\.\d0*)|(0\.[1..9]\d?0*))[kM]?$'
        }
    } 
}

_GEN_FILES_FOLDER_PREFIX = os.path.join(os.path.dirname(__file__),"core/output")

def get_params_def(script_id):
    obj = _SCRIPTS_DEF[script_id] 
    return obj


def gen_files(script_id, config):
    output_dir = partomatic.web_generate(script_id, config, _GEN_FILES_FOLDER_PREFIX) 
    return output_dir
