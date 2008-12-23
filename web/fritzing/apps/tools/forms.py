from django import forms
from django.db import models
from django.conf import settings
from django.core.exceptions import ImproperlyConfigured
from django.utils.translation import ugettext_lazy as _

from contact_form.forms import AkismetContactForm

# favour django-mailer but fall back to django.core.mail
try:
    mailer = models.get_app("mailer")
    from mailer import send_mail
except ImproperlyConfigured:
    from django.core.mail import send_mail
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

    def save(self, fail_silently=False):
        send_mail(**self.get_message_dict())
