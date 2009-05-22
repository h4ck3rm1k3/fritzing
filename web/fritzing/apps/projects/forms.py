from django import forms
from django.utils.translation import ugettext_lazy as _
from projects.models import Project, Resource, Image, Attachment, Category
from markitup.widgets import MarkItUpWidget
from template_utils.markup import formatter
from django.forms.widgets import TextInput, MultiWidget, HiddenInput, FileInput
from django.forms.models import ModelChoiceField

RESOURCE_DELIMITER = '########'

class ResourceMultiWidget(forms.MultiWidget):
    def __init__(self, attrs=None):
        widgets = (TextInput(attrs), TextInput(attrs))
        super(ResourceMultiWidget,self).__init__(widgets, attrs)

    def decompress(self, value):
        if value:
            return value.split(RESOURCE_DELIMITER)
        return [None, None]

class ResourceField(forms.MultiValueField):
    widget=ResourceMultiWidget

    def compress(self, data_list):
        if data_list:
            return RESOURCE_DELIMITER.join(data_list)
        return None

class FileMultiWidget(forms.MultiWidget):
    def __init__(self, attrs=None):
        widgets = (HiddenInput(attrs), FileInput(attrs))
        super(FileMultiWidget,self).__init__(widgets, attrs)

    def decompress(self, value):
        if value:
            return value.split(RESOURCE_DELIMITER)
        return [None, None]

class MultiFileField(forms.MultiValueField):
    widget=FileMultiWidget

    def compress(self, data_list):
        if data_list:
            return RESOURCE_DELIMITER.join(data_list)
        return None

class MultiFileInput(FileInput):
    def render(self, name, value, attrs=None):
        extra_attrs = {'class': 'multi'}
        extra_attrs.update(attrs)
        return super(MultiFileInput, self).render(name, None, attrs=extra_attrs)

class ProjectForm(forms.ModelForm):
    title = forms.CharField(
        help_text=_('The main title to be used '),
        label=_('Title'))

    description = forms.CharField(
        label=_('Description'),
        widget=MarkItUpWidget(
            attrs={'rows': '4'},
            markitup_set='projects/sets/projects'))

    instructions = forms.CharField(
        label=_('Instructions'),
        widget=MarkItUpWidget(
            markitup_set='projects/sets/projects'))

    main_image = forms.ImageField()
        # widget=MultiFileInput({
        #     'maxlength': '1'}))

    fritzing_files = forms.FileField()

    category = ModelChoiceField(
        required=False,
        queryset=Category.objects.all())

    other_files = MultiFileField(required=False)

    other_images = forms.ImageField(required=False)
        # widget=MultiFileInput({
        #     'list': '#main_image_selection',
        #     'accept': 'gif|jpeg|jpg|png'}))

    links = ResourceField(
        required=False,
        label=_('External links'),
        fields=(
            forms.CharField(),
            forms.URLField()))

    class Meta:
        model = Project
        fields= (
            'title',
            'description',
            'instructions',
            'difficulty',
            'tags',
            'license'
        )
