from fritzing.parts_gen.core import partomatic

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
            'regex': '^[1..9]\d*[kM]?$'
        }
    } 
}

_GEN_FILES_FOLDER_PREFIX = \
    '/home/merun/workspace/fritzing/web/parts_gen/src/fritzing/parts_gen/core/output'

def get_params_def(script_id):
    obj = _SCRIPTS_DEF[script_id] 
    return obj


def gen_files(script_id, config):
    output_dir = partomatic.web_generate(script_id, config, _GEN_FILES_FOLDER_PREFIX) 
    return output_dir
