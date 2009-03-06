from django import forms
        
class ResistorDynamicForm(forms.Form):
    script_id = forms.CharField(widget=forms.HiddenInput, initial='resistor') 
    resistance = forms.IntegerField(required=True, label='Choose the resistance', min_value = 1) 
    unit = forms.ChoiceField(required=True, label='Choose the unit', choices=[('kv', 'kl'), ('mv', 'ml')]) 
    
    def __init__(self, *args, **kwargs):
        super(ResistorDynamicForm, self).__init__(*args, **kwargs)

    