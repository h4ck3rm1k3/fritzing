from django import forms
        
class Generic_male_headerDynamicForm(forms.Form):
    script_id = forms.CharField(widget=forms.HiddenInput, initial='generic-male-header') 
    title = forms.CharField(required=True, label='Part title', initial='Generated Generic Male Header') 
    pins = forms.ChoiceField(required=True, label='Amount of pins', choices=[(1, 1), (2, 2), (3, 3), (4, 4), (5, 5), (6, 6), (7, 7), (8, 8), (9, 9), (10, 10), (11, 11), (12, 12), (14, 14), (16, 16), (18, 18), (20, 20)]) 
    
    def __init__(self, *args, **kwargs):
        super(Generic_male_headerDynamicForm, self).__init__(*args, **kwargs)

    