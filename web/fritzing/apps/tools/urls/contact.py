from django.conf.urls.defaults import *
from django.views.generic.simple import direct_to_template

from contact_form.views import contact_form
from tools.forms import FritzingContactForm

urlpatterns = patterns('',
                       url(r'^$',
                           contact_form,
                           {'form_class': FritzingContactForm},
                           name='contact_form'),
                       url(r'^sent/$',
                           direct_to_template,
                           { 'template': 'contact_form/contact_form_sent.html' },
                           name='contact_form_sent'),
                       )
