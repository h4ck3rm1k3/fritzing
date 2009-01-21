from django.contrib import admin    

from forum.models import Thread, Post
from forum.admin import ThreadAdmin

class PostInlineAdmin(admin.TabularInline):
    model = Post

class InlineThreadAdmin(ThreadAdmin):
    inlines = (PostInlineAdmin,)

admin.site.unregister(ThreadAdmin)
admin.site.register(Thread, InlineThreadAdmin)
