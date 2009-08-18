import ticker
from django.contrib import admin
from fritzing.apps.news import models
from django import forms
from tinymce.widgets import TinyMCE

class NewsEntryForm(forms.ModelForm):
    class Meta:
        model = models.NewsEntry
        
    content = forms.CharField(widget=TinyMCE(attrs={'cols': 30, 'rows': 30}))
    content_more = forms.CharField(widget=TinyMCE(attrs={'cols': 30, 'rows': 30}),required=False)

class NewsEntryAdmin(ticker.admin.EntryAdmin):
    form = NewsEntryForm
    fields = (
        'author',
        'status',
        'title',
        'date',
        'content',
        'content_more',
        'tags',
        'enable_comments',
    )
    
    
    
admin.site.register(models.NewsEntry, NewsEntryAdmin)