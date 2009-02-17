from django import forms
from django.conf import settings
from django.utils.translation import ugettext_lazy as _

from contact_form.forms import AkismetContactForm

# favour django-mailer but fall back to django.core.mail
if "mailer" in settings.INSTALLED_APPS:
    from mailer import send_mail
else:
    from django.core.mail import send_mail
attrs = {'class': 'required'}

class FritzingContactForm(AkismetContactForm):
    recipient_list = AkismetContactForm.recipient_list + ['info@fritzing.org']
    message_subject = forms.CharField(
        max_length=100,
        widget=forms.TextInput(attrs=attrs),
        label=_('Message subject')
    )

    def subject(self):
        return settings.EMAIL_SUBJECT_PREFIX + self.cleaned_data["message_subject"]

    def message(self):
        return "From: %(name)s <%(email)s>\n\n%(body)s" % self.cleaned_data
