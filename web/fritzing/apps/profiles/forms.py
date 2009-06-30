from django.conf import settings
from django import forms

from fritzing.apps.profiles.models import Profile

class ProfileForm(forms.ModelForm):

    class Meta:
        model = Profile
        exclude = ('user',)
