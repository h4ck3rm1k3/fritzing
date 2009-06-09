from django.contrib import admin
from django import forms
import dregni
from tinymce.widgets import TinyMCE
from fritzing.apps.events import models

class EventForm(forms.ModelForm):
    class Meta:
        model = models.Event
        
    body = forms.CharField(widget=TinyMCE(attrs={'cols': 30, 'rows': 30}))
    description = forms.CharField(max_length=100,widget=forms.TextInput(attrs={'size': 94}))

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
