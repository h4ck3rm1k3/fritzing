from os import path
import sys

def string_field(name,obj):
    src = '''
    %(name)s = forms.CharField(required=True%(attrs)s) ''' \
    % {'name' : name, 'attrs' : get_label_err_msgs_and_initial(obj) }
    return src
    
def int_field(name,obj):
    src = '''
    %(name)s = forms.IntegerField(required=True%(attrs)s) ''' \
    % {'name' : name, 'attrs' : get_label_err_msgs_and_initial(obj)+get_range(obj) }
    return src

    
def float_field(name,obj):
    src = '''
    %(name)s = forms.FloatField(required=True%(attrs)s) ''' \
    % {'name' : name, 'attrs' : get_label_err_msgs_and_initial(obj)+get_range(obj) }
    return src

    
def enum_field(name,obj):
    src = '''
    %(name)s = forms.ChoiceField(required=True%(attrs)s, choices=%(choices)s) ''' \
    % {'name' : name, 'attrs' : get_label_err_msgs_and_initial(obj), 'choices' : obj['choices'] }
    return src


def regex_field(name,obj):
    src = '''
    %(name)s = forms.RegexField(required=True%(attrs)s) ''' \
    % {'name' : name, 'attrs' : get_label_err_msgs_and_initial(obj)+get_regex(obj) }
    return src

    
def get_range(obj):
    src  = get_aux(obj,'min_value',"")
    src += get_aux(obj,'max_value',"")
    return src
        
def get_label_err_msgs_and_initial(obj):
    return get_aux(obj,'label')\
        +get_aux(obj,'error_messages','')\
        +get_aux(obj,'initial')\

def get_regex(obj):
    return get_aux(obj,'regex')

def get_aux(obj,prop_name,separator="'"):
    src = ""
    if obj.has_key(prop_name):
        src = ", %(name)s=%(sep)s%(value)s%(sep)s" % {'name':prop_name, 'value':obj[prop_name], 'sep': separator}
    return src

        
field_types = {
    'string': string_field,
    'int'   : int_field,
    'float' : float_field,
    'enum'  : enum_field,
    'regex' : regex_field 
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
    
        for name in params.iterkeys():
            value = params[name]
            print name
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
