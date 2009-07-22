from django import forms
from django.utils.translation import ugettext_lazy as _
from fritzing.apps.projects.models import Project, Resource, Image, Attachment, Category
from markitup.widgets import MarkItUpWidget
from template_utils.markup import formatter
from django.forms.widgets import TextInput, HiddenInput, FileInput, Textarea
from django.forms.util import ValidationError
from django.forms.fields import URLField
from licenses.models import License
import re, urlparse 

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
    
    def __init__(self, *args, **kwargs):
        fields = (
            forms.CharField(),
            forms.URLField(),
        )
        super(ResourceField, self).__init__(fields, *args, **kwargs)

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
    
def _get_default_license():
    shareAlike = License.objects.filter(is_active=True,abbreviation='CC-BY-SA')
    if(shareAlike.count() > 0):
        return shareAlike[0].id
    else: return 0

class ProjectForm(forms.ModelForm):
    def _init_file_field(self, field_name):
        new_values = [x.name for x in self.files.getlist(field_name)] if self.files else []
        deleted_values = self.data.getlist('deleted_%s'%field_name) if self.data else []      
        prev_values = self.data.getlist('prev_%s'%field_name) if self.data else []
        
        self.file_fields[field_name] = {}
        self.file_fields[field_name]['new_values'] = new_values
        self.file_fields[field_name]['deleted_values'] = deleted_values
        self.file_fields[field_name]['prev_values'] = prev_values
    
    def __init__(self, *args, **kwargs):
        super(forms.ModelForm,self).__init__(*args, **kwargs)
        
        if args and u'links_title' in args[0].keys():
            self.resources_title = args[0].getlist('links_title')
            
        if args and u'links_url' in args[0].keys():
            self.resources_url = args[0].getlist('links_url')
        
        if 'instance' in kwargs:
            instance = kwargs['instance'] 
            if instance.category:
                self.fields['category'].initial = instance.category.id
                
        
        file_fields_aux = ['main_image','fritzing_files','code','examples','other_images']
        for ff in file_fields_aux:
            self._init_file_field(ff)

    
    file_fields = {}
        
    title = forms.CharField(
        help_text=_('The title of your project'),
        label=_('Title'))
    
    id = forms.IntegerField(
        widget=forms.HiddenInput(),
        required=False
    )

    description = forms.CharField(
        label=_('Description'),
        help_text=_('A very short description/subheadline of what your project is about'),
        widget=Textarea(attrs={'rows': '3'})
    )

    instructions = forms.CharField(
        label=_('Body'),
        help_text=_('Give a detailed description of your project, including instructions on how to build it'),
        widget=MarkItUpWidget(
            markitup_set='projects/sets/projects'))

    main_image = forms.ImageField(
        required=False,
        label=_('Header Image'),
        help_text=_('A representative screenshot or photograph of your project'),
        widget=forms.FileInput(attrs={'size':'35'})
    )

    fritzing_files = forms.FileField(
        required=False,
        help_text=_('Upload your Fritzing sketch (.fz) and other Fritzing files here'),
        widget=forms.FileInput(attrs={'size':'35'})
    )

    code = forms.FileField(
        required=False,
        help_text=_('Add any code that is part of your project (e.g., for the Arduino)'),
    )
    
    examples = forms.FileField(
        label=_('Other Files'),
        required=False,
        help_text=_('Add any other files that help documenting your project'),
    )

    other_images = forms.ImageField(
        required=False,
        label=_('Image Gallery'),
        help_text=_('Add a set of images and photos')
    )
        
    resource = ResourceField(
        required=False,
        label=_('External links'),
        help_text=_('Some links to external websites related to your project')
    )
    
    resources_title = []
    resources_url = []
    
    license = forms.ModelChoiceField(
        empty_label=None,
        initial=_get_default_license(),
        queryset=License.objects.filter(is_active=True),
        help_text=_("""
Pick a license for your documentation
(<a target="_blank" href="http://creativecommons.org/licenses/">More about CC licenses</a>)<br/>
<ul>
    <li><a target="_blank" href="http://creativecommons.org/licenses/by-sa/3.0/">
    Creative Commons Attribution Share-Alike (default)
    </a></li>
    <li><a target="_blank" href="http://creativecommons.org/licenses/by-nc-sa/3.0/">
    Creative Commons Attribution Non-Commercial Share-Alike
    </a></li>
</ul> 
        """)
    )
    
        
    '''
    HELP FUNCTIONS TO DEAL WITH FILES
    '''
    def _eq_lists(self,list1, list2):
        if len(list1) != len(list2): return False
        else:
            for l1 in list1:
                if l1 not in list2: return False
            return True
        
    def _is_empty(self,list): return not list
        
    def _something_changed(self,new_values, deleted_values):
        return not self._is_empty(new_values) \
            or not self._is_empty(deleted_values)
    
    def something_changed(self, field):
        return self._something_changed(
            self.file_fields[field]['new_values'],
            self.file_fields[field]['deleted_values']
        )
        
    def _all_removed(self,prev_values, deleted_values):
        return self._eq_lists(prev_values,deleted_values)
    
    def all_removed(self,field):
        return self._all_removed(
            self.file_fields[field]['prev_values'],
            self.file_fields[field]['deleted_values']
        )
            
    def _something_added(self,new_values):
        return not self._is_empty(new_values)
    
    def removed(self,field):
        return self.file_fields[field]['deleted_values']

    
    def non_deleted_previous_files(self,field):
        list1 = self.file_fields[field]['prev_values']
        list2 = self.file_fields[field]['deleted_values']
        return [item for item in list1 if not item in list2]
        
    
    def _clean_file_aux(self,field_name,required=False):
        data = self.cleaned_data[field_name]
        new_values = self.file_fields[field_name]['new_values']
        deleted_values = self.file_fields[field_name]['deleted_values']
        prev_values = self.file_fields[field_name]['prev_values']
        
        if self._all_removed(prev_values, deleted_values) \
            and not self._something_added(new_values) \
            and required:
                raise ValidationError(self.fields[field_name].error_messages['required'])
    
        return data

    
    def clean_main_image(self):
        return self._clean_file_aux('main_image',required=True)
        
    def clean_fritzing_files(self):
        return self._clean_file_aux('fritzing_files',required=True)
    
    def clean_code(self):
        return self._clean_file_aux('code')
    
    def clean_examples(self):
        return self._clean_file_aux('examples')

    def clean_other_images(self):
        return self._clean_file_aux('other_images')

    def clean_title(self):
        title = self.cleaned_data['title']        
        proys = Project.objects.filter(title=title)
        
        for proy in proys:
            if not self.data['id'] or proy.id != int(self.data['id']):
                raise ValidationError(_('There\'s already a project with that title, please choose a new one'))

        return title
    
    def clean_id(self):
        # don't want this value to populate the project instance
        return None
    
    def clean_tags(self):
        return self.cleaned_data['tags'].lower()
            
    
    def clean_resource(self):
        indexes_to_remove = []
        
        print self.resources_title
        
        # if the whole field was not binded, just remove it from the data to validate
        for i in range(len(self.resources_url)):
            if self.resources_url[i].strip() == u'':
                if self.resources_title[i].strip() != u'':
                    raise ValidationError(_('If the title is defined, the url must be defined as well'))
                else:
                    indexes_to_remove.append(i)
                
        for i in indexes_to_remove:
            del self.resources_title[i]
            del self.resources_url[i]
        
        # now, let's validate the urls
        final_urls = []
        for url in self.resources_url:            
            #url = URLField(required=True,verify_exists=True).clean(url)
            final_urls.append(URLField().clean(url))        
        
        # if a title was not provided, but its url was, populate it whit the same value as the url
        final_titles = []
        idx = 0
        for title in self.resources_title:
            final_titles.append(final_urls[idx] if title.strip() == u'' else title)
            idx=idx+1
        
        self.resources =  [(final_titles[i],final_urls[i]) for i in range(len(final_titles))]
        print self.resources
        
        return ''

    class Meta:
        model = Project
        fields= (
            'id',
            'title',
            'description',
            'instructions',
            'difficulty',
            'tags',
            'license',
            'category',
        )
