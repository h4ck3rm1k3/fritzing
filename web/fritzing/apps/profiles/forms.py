from django import forms

from fritzing.apps.profiles.models import Profile
#from django.contrib.admin import widgets
from tinymce.widgets import TinyMCE

class ProfileForm(forms.ModelForm):
    user_first_name = forms.CharField()
    user_last_name = forms.CharField()
    user_email = forms.EmailField(required=False)
    #image = forms.ImageField(widget=widgets.AdminFileWidget())
    about = forms.CharField(required=False,widget=TinyMCE(attrs={'cols': 50, 'rows': 30}))
    
    def __init__(self, *args, **kwargs):
        forms.ModelForm.__init__(self, *args, **kwargs)
        user_aux = kwargs['instance'].user
        self.fields['user_first_name'].initial = user_aux.first_name
        self.fields['user_last_name'].initial = user_aux.last_name
        self.fields['user_email'].initial = user_aux.email

    class Meta:
        model = Profile
        exclude = ('user',)
        #fields = ('user_first_name', 'user_first_name', 'user_email', 'location', 'website', 'image')

