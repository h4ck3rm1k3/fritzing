from django.forms.models import ModelForm
from django.contrib import admin    
from django import forms
from forum.models import Thread, Post
from forum.admin import ThreadAdmin

class PostInlineAdmin(admin.TabularInline):
    model = Post

class InlineThreadAdmin(ThreadAdmin):
    inlines = (PostInlineAdmin,)

admin.site.unregister(Thread)
admin.site.register(Thread, InlineThreadAdmin)

from ticker.models import Entry
from ticker.admin import EntryAdmin
from tinymce.widgets import TinyMCE

admin.site.unregister(Entry)

class EntryForm(ModelForm):
    content = forms.CharField(widget=TinyMCE(attrs={'cols': 80, 'rows': 30}))
    content_more = forms.CharField(widget=TinyMCE(attrs={'cols': 80, 'rows': 30}))

    class Meta:
        model = Entry

class TinyMCEEntryAdmin(EntryAdmin):
    form = EntryForm

admin.site.register(Entry, TinyMCEEntryAdmin)
