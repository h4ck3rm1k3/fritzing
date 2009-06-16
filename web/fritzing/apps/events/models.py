from django.db import models
from django.utils.translation import ugettext_lazy as _
import dregni.models
from datetime import date

class Event(dregni.models.Event):
    body = models.TextField(_('body'))

    def has_past(self):
        return self.start_date < date.today()
    
class EventWhere(models.Model):
    class Meta:
        verbose_name = 'Where'
        verbose_name_plural = 'Where'
        
    text = models.CharField(_('text'), max_length=255)
    url = models.URLField(_('URL'), blank=True, verify_exists=False)
    event = models.OneToOneField(Event,related_name='where',verbose_name=_('Where'),unique=True,blank=False,null=False)
    
class EventLink(models.Model):
    class Meta:
        verbose_name = 'Event main page'
        verbose_name_plural = 'Event main page'
        
    text = models.CharField(_('text'), max_length=255)
    url = models.URLField(_('URL'))
    event = models.OneToOneField(Event,related_name='link',verbose_name=_('Event main page'),unique=True,blank=False,null=False)
    