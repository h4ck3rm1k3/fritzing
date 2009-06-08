from django.db import models
from django.utils.translation import ugettext_lazy as _
import dregni.models

class Event(dregni.models.Event):
    body = models.TextField(_('body'))
