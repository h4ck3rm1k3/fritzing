SCRIPT_CONFS = {
    'resistor' : {
        'name_param' : 'resistance', # the parameter used to generate the part name and file
        'templates' : { # file names, without extension
            'breadboard' : 'basic-resistor_bread',
            'icon'       : 'basic-resistor_icon',
            ''           : 'basic-resistor'
        }
    },
    'mystery' : {
        'name_param' : 'pins', # the parameter used to generate the part name and file
        'templates' : { # file names, without extension
            'breadboard' : 'mystery-part_bread',
            'schematic'  : 'mystery-part_schem',
            ''           : 'mystery-part'
        }
    },
    'generic-male-header' : {
        'name_param' : 'pins', # the parameter used to generate the part name and file
        'templates' : { # file names, without extension
            'breadboard' : 'generic-male-header_bread',
            'schematic'  : 'generic-male-header_schem',
            ''           : 'generic-male-header'
        }
    },
    'generic-female-header' : {
        'name_param' : 'pins', # the parameter used to generate the part name and file
        'templates' : { # file names, without extension
            'breadboard' : 'generic-female-header_bread',
            'schematic'  : 'generic-female-header_schem',
            ''           : 'generic-female-header'
        }
    },
    'generic-female-header-rounded' : {
        'name_param' : 'pins', # the parameter used to generate the part name and file
        'templates' : { # file names, without extension
            'breadboard' : 'generic-female-header-rounded_bread',
            'schematic'  : 'generic-female-header-rounded_schem',
            ''           : 'generic-female-header-rounded'
        }
    },
}
