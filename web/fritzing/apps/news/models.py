import ticker
from datetime import datetime
from django.db import models
from django.utils.translation import ugettext as _

class NewsEntry(ticker.models.Entry):
    date = models.DateTimeField(_('date'), default=datetime.now())
    
    objects = ticker.models.EntryManager()
    
    class Meta:
        ordering = ('-date',)
        verbose_name = _('entry')
        verbose_name_plural = _('entries')

    