from django.db import models
from django.utils.translation import ugettext_lazy as _
import dregni.models
from datetime import date

class Event(dregni.models.Event):
    body = models.TextField(_('body'))

    def has_past(self):
        return self.start_date < date.today()
    
class EventWhere(models.Model):
    text = models.CharField(_('text'), max_length=255)
    url = models.URLField(_('URL'), blank=True, verify_exists=False)
    event = models.OneToOneField(Event,related_name='where',verbose_name=_('Where'),unique=True,blank=False,null=False)
    
class EventLink(models.Model):
    text = models.CharField(_('text'), max_length=255)
    url = models.URLField(_('URL'))
    event = models.OneToOneField(Event,related_name='link',verbose_name=_('Event main page'),unique=True,blank=False,null=False)
    