import tempfile, zipfile, shutil
from django.http import HttpResponse
from django.shortcuts import render_to_response
from django.core.servers.basehttp import FileWrapper
from django.conf import settings
from django.template import RequestContext
from fritzing.apps.parts.forms import generator
from fritzing.apps.parts.utils import script_config_from_form, add_folder_to_zipfile
from fritzing.apps.parts.dispatcher import get_params_def, gen_files
from fritzing.apps.parts.core.defs import AVAIL_SCRIPTS

def choose(request):
    return render_to_response(
        'parts/generator/choose.html',
        {'scripts_list': AVAIL_SCRIPTS, 'debugging' : settings.DEBUG},
        context_instance = RequestContext(request)
    )

def form(request): 
    script_id = request.POST['script_id']
    params = get_params_def(script_id)
    
    class_name = generator.create_class_if_needed(params, script_id, settings.DEBUG)
    form = generator.get_form_class(class_name)()
    return render_to_response(
        'parts/generator/form.html',
        {'form': form},
        context_instance = RequestContext(request)
    )


def send_zipfile(script_id,config):
    """                                                                         
    Create a ZIP file on disk and transmit it in chunks of 8KB,                 
    without loading the whole file into memory.
    (http://www.djangosnippets.org/snippets/365/)                                 
    """
    
    files_location, file_suffix = gen_files(script_id, config)

    temp = tempfile.TemporaryFile()
    archive = zipfile.ZipFile(temp, 'w', zipfile.ZIP_DEFLATED)
    add_folder_to_zipfile(files_location, archive, files_location) 
    archive.close()
    
    # clean up generated files
    shutil.rmtree(files_location)
    
    wrapper = FileWrapper(temp)
    response = HttpResponse(wrapper, content_type='application/Fritzing')
    response['Content-Disposition'] = \
        'attachment; filename=gen_part_%(s_id)s_%(suffix)s.fzpz' % {'s_id' : script_id, 'suffix' : file_suffix} 
    response['Content-Length'] = temp.tell()
    temp.seek(0)
    
    return response


def generate(request):
    if request.method == 'POST': # If the form has been submitted...
        script_id = request.POST['script_id']
        class_name = generator.get_class_name(script_id)
        form = generator.get_form_class(class_name)(request.POST)
        if form.is_valid(): # All validation rules pass 
            config, script_id = script_config_from_form(form.cleaned_data)
            return send_zipfile(script_id,config)
        else:
            return render_to_response(
                'parts/generator/choose.html',
                {'script_form': form,
                 'script_id' : script_id,
                 'scripts_list': AVAIL_SCRIPTS,
                 'debugging' : settings.DEBUG},
                context_instance = RequestContext(request)
            )
    else:
        return HttpResponse('no form posted')
    
