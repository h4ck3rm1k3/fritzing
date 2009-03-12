from django.http import HttpResponse
from django.shortcuts import render_to_response
from fritzing.parts_gen.forms import gen
import tempfile, zipfile, shutil
from django.core.servers.basehttp import FileWrapper
from fritzing import settings
from fritzing.parts_gen.utils import \
script_config_from_form, add_folder_to_zipfile
from fritzing.parts_gen.dispatcher import \
AVAIL_SCRIPTS, get_params_def, gen_files


def choose(request):
    return render_to_response('parts_gen/choose.html', {'scripts_list': AVAIL_SCRIPTS})

def form(request): 
    script_id = request.POST['script_id']
    params = get_params_def(script_id)
    
    class_name = gen.create_class_if_needed(params, script_id, settings.DEBUG)
    form = gen.get_form_class(class_name)()
    return render_to_response('parts_gen/form.html', {'form': form})


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
        class_name = gen.get_class_name(script_id)
        form = gen.get_form_class(class_name)(request.POST)
        if form.is_valid(): # All validation rules pass 
            config, script_id = script_config_from_form(form.cleaned_data)
            return send_zipfile(script_id,config)
        else:
            return render_to_response('parts_gen/form.html', {'form': form})
    else:
        return HttpResponse('no form posted')
    
