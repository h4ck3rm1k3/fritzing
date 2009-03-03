from django import forms

def string_field(obj):
    field = forms.CharField(required=True)
    set_label(field,obj)
    return field
    
def int_field(obj):
    field = forms.IntegerField(required=True)
    set_label(field,obj)
    set_range(field,obj)
    return field
    
def float_field(obj):
    field = forms.FloatField(required=True)
    set_label(field,obj)
    set_range(field,obj)
    return field
    
def enum_field(obj):
    field = forms.ChoiceField(required=True)
    set_label(field,obj)
    field.choices = obj['options']
    return field
    
def set_range(field,obj):
    if obj.has_key('range_min'):
        field.min_value = obj['range_min']
    if obj.has_key('range_max'):
        field.min_value = obj['range_max']
        
def set_label(field,obj):
    if obj.has_key('label'):
        field.label = obj['label'] 

class DynamicForm(forms.Form):
    field_types = {
        'string': string_field,
        'int'   : int_field,
        'float' : float_field,
        'enum'  : enum_field
    }
        
    def __init__(self, *args, **kwargs):
        super(DynamicForm, self).__init__(*args, **kwargs)
            
    def load_fields(self,params, script_id):
        for name, value in params.iteritems():
            self.fields[name] = self.get_field_from_obj(value)
        self.fields['script_id'] = forms.CharField(widget=forms.HiddenInput,initial=script_id)

    def get_field_from_obj(self,obj):
        f = self.field_types[obj['type']]
        return f(obj)
            