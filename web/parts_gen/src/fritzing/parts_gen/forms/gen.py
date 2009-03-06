from os import path
import sys

def string_field(name,obj):
    src = '''
    %(name)s = forms.CharField(required=True%(attrs)s) ''' \
    % {'name' : name, 'attrs' : get_label(obj) }
    return src
    
def int_field(name,obj):
    src = '''
    %(name)s = forms.IntegerField(required=True%(attrs)s) ''' \
    % {'name' : name, 'attrs' : get_label(obj)+get_range(obj) }
    return src

    
def float_field(name,obj):
    src = '''
    %(name)s = forms.FloatField(required=True%(attrs)s) ''' \
    % {'name' : name, 'attrs' : get_label(obj)+get_range(obj) }
    return src

    
def enum_field(name,obj):
    src = '''
    %(name)s = forms.ChoiceField(required=True%(attrs)s, choices=%(choices)s) ''' \
    % {'name' : name, 'attrs' : get_label(obj), 'choices' : obj['options'] }
    return src
    
def get_range(obj):
    src=""
    if obj.has_key('range_min'):
        src += ", min_value = %s" % obj['range_min']
    if obj.has_key('range_max'):
        src += ", max_value = %s" % obj['range_max']
    return src
        
def get_label(obj):
    if obj.has_key('label'):
        return ", label='%s'" % obj['label']
    return ""
        
field_types = {
    'string': string_field,
    'int'   : int_field,
    'float' : float_field,
    'enum'  : enum_field
}

def get_field_from_obj(name,obj):
    f = field_types[obj['type']]
    return f(name,obj)


def get_class_name(script_id):
    return script_id.capitalize()+"DynamicForm"


def get_form_class(class_name):
    name = 'fritzing.parts_gen.forms.'+class_name
    __import__(name)
    module = sys.modules[name]

    return getattr(module, class_name)

        
def create_class_if_needed(params, script_id, force=False):
    class_name = get_class_name(script_id)
    dirpath = path.dirname(__file__)
    file_name = dirpath+"/"+class_name+".py"
    
    if force or not path.exists(file_name):
        src = \
'''from django import forms
        
class %(class_name)s(forms.Form):
    script_id = forms.CharField(widget=forms.HiddenInput, initial='%(script_id)s') '''
    
        for name, value in params.iteritems():
            src += get_field_from_obj(name,value)
        src += '''
    
    def __init__(self, *args, **kwargs):
        super(%(class_name)s, self).__init__(*args, **kwargs)

    '''
    
        src = src % {'script_id' : script_id, 'class_name' : class_name}
    
        file = open(file_name,'w')
        file.write(src)
        file.close()
        
    return class_name
