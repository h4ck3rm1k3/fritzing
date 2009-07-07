from django import forms

from fritzing.apps.profiles.models import Profile

class ProfileForm(forms.ModelForm):
    user_first_name = forms.CharField()
    user_last_name = forms.CharField()
    user_email = forms.EmailField()
    
    def __init__(self, *args, **kwargs):
        forms.ModelForm.__init__(self, *args, **kwargs)
        user_aux = kwargs['instance'].user
        self.fields['user_first_name'].initial = user_aux.first_name
        self.fields['user_last_name'].initial = user_aux.last_name
        self.fields['user_email'].initial = user_aux.email

    class Meta:
        model = Profile
        fields = ('user_first_name', 'user_first_name', 'user_email', 'location', 'website', 'image')

