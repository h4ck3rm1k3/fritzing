from django import forms
from django.contrib import admin
from fritzing.apps.projects.models import Category, Project, Resource, Image, Attachment
from pages.admin.widgets import markItUpMarkdown
from markitup.widgets import MarkItUpWidget
from reversion.admin import VersionAdmin

class ResourceInlineAdmin(admin.TabularInline):
    model = Resource

class ImageAdmin(admin.TabularInline):
    model = Image
    exclude = ('user',)

    def save_form(self, request, form, change):
        instance = super(ImageAdmin, self).save_form(request, form, change)
        if not change:
            instance.user = request.user
        return instance

class AttachmentAdmin(admin.TabularInline):
    model = Attachment
    exclude = ('user',)

    def save_form(self, request, form, change):
        instance = super(AttachmentAdmin, self).save_form(request, form, change)
        if not change:
            instance.user = request.user
        return instance

markdown_help_text='Markdown formatting allowed, see http://en.wikipedia.org/wiki/Markdown for syntax or use editor. No HTML allowed.'

class ProjectAdminForm(forms.ModelForm):
    description = forms.CharField(widget=MarkItUpWidget(), help_text=markdown_help_text)
    instructions = forms.CharField(widget=MarkItUpWidget(), required=False, help_text=markdown_help_text)

    class Meta:
        model = Project

class ProjectAdmin(VersionAdmin):
    form = ProjectAdminForm
    model = Project
    exclude = ('author',)
    list_display = ('title', 'slug', 'author', 'difficulty', 'featured', 'blessed', 'public')
    filter_horizontal = ('members',)
    inlines = [
        ResourceInlineAdmin,
        AttachmentAdmin,
        ImageAdmin,
    ]

    def save_form(self, request, form, change):
        instance = super(ProjectAdmin, self).save_form(request, form, change)
        if not change:
            instance.author = request.user
        return instance

    def save_formset(self, request, form, formset, change):
        formset.save()
        for attachment in form.instance.attachments.all():
            attachment.user = request.user
            attachment.save()

class CategoryAdminForm(forms.ModelForm):
    description = forms.CharField(widget=MarkItUpWidget(), required=False, help_text=markdown_help_text)

    class Meta:
        model = Project

class CategoryAdmin(admin.ModelAdmin):
    list_display = ('level_and_title',)
    form = CategoryAdminForm
    model = Category

admin.site.register(Category, CategoryAdmin)
admin.site.register(Project, ProjectAdmin)
