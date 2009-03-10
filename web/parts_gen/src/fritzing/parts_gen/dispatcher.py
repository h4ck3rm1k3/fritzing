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
            'type' : 'int',
            'min_value' : 1
        },
        'unit' : {
            'label': 'Choose the unit',
            'type' : 'enum',
            'options' : [('kv','kl'),('mv','ml')]
        }
    } 
}

_GEN_FILES_FOLDER_PREFIX = '/home/merun/workspace/fritzing/web/parts_gen/parts/'

def get_params_def(script_id):
    obj = _SCRIPTS_DEF[script_id] 
    return obj

def gen_files(script_id, config):
    # TODO: here execute script, and generate svg and fzp files 
    # inside a folder, inside the _GEN_FILES_FOLDER_PREFIX folder 
    return _GEN_FILES_FOLDER_PREFIX
