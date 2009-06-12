from django.contrib import admin
from django import forms
import dregni
from tinymce.widgets import TinyMCE
from fritzing.apps.events import models

class EventWhereInLineForm(forms.ModelForm):
    class Meta:
        model = models.EventWhere
    
    def clean_text(self):
        raise forms.ValidationError("The field %s cannot be empty" % field_name)
        return self.clean_aux('text')
    
    def clean_url(self):
        raise forms.ValidationError("The field %s cannot be empty" % field_name)
        return self.clean_aux('url')
    
    def clean_aux(self,field_name):
        data = self.cleaned_data[field_name]
        #if(data.strip() == ""):
        return data

class EventWhereInline(admin.TabularInline):
    model = models.EventWhere
    #form = EventWhereInLineForm
    

class EventForm(forms.ModelForm):
    class Meta:
        model = models.Event
        
    body = forms.CharField(widget=TinyMCE(attrs={'cols': 30, 'rows': 30}))
    description = forms.CharField(max_length=100,widget=forms.TextInput(attrs={'size': 94}))


class EventAdmin(dregni.admin.EventAdmin):
    form = EventForm
    inlines = [EventWhereInline]
    fieldsets = (
        (None, {
            'fields': (('title', 'slug'), 'description', 'body', 'tags'),
        }),
        ('Scheduling', {
            'fields': (('start_date', 'start_time'), ('end_date', 'end_time')),
        })
    )
    search_fields = ('title', 'description', 'body', 'tags')
    
admin.site.register(models.Event, EventAdmin)
