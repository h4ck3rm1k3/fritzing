from django.contrib import admin
from django import forms
import dregni
from tinymce.widgets import TinyMCE
from fritzing.apps.events import models

class EventForm(forms.ModelForm):
    class Meta:
        model = models.Event
        
    body = forms.CharField(widget=TinyMCE(attrs={'cols': 20, 'rows': 60}))
    description = forms.CharField(max_length=100,widget=forms.Textarea(attrs={'cols': 80, 'rows': 2}))

class EventAdmin(dregni.admin.EventAdmin):
    form = EventForm
    fieldsets = (
        (None, {
            'fields': (('title', 'slug'), 'description', 'body', 'tags'),
        }),
        ('Scheduling', {
            'fields': (('start_date', 'start_time'), ('end_date', 'end_time')),
        }),
    )
    search_fields = ('title', 'description', 'body', 'tags')
    
admin.site.register(models.Event, EventAdmin)
