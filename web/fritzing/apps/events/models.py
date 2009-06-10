from django.db import models
from django.utils.translation import ugettext_lazy as _
import dregni.models
from datetime import date 

class Event(dregni.models.Event):
    body = models.TextField(_('body'))
    _where = None
    
    def where(self):
        if not self._where:
            for md in self.metadata.all():
                if md.type.title == "where":
                    self._where = md
        return self._where

    def has_past(self):
        return self.start_date < date.today()
