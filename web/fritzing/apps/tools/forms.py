from django import forms
from django.conf import settings
from django.utils.translation import ugettext_lazy as _

from contact_form.forms import AkismetContactForm

attrs = {'class': 'required'}

class FritzingContactForm(AkismetContactForm):
    message_subject = forms.CharField(
        max_length=100,
        widget=forms.TextInput(attrs=attrs),
        label=_('Message subject')
    )

    def subject(self):
        return settings.EMAIL_SUBJECT_PREFIX + self.cleaned_data["message_subject"]

    def message(self):
        return "From: %(name)s <%(email)s>\n\n%(body)s" % self.cleaned_data
