import ticker
from django.contrib import admin
from fritzing.apps.news import models

class NewsEntryAdmin(ticker.admin.EntryAdmin):
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