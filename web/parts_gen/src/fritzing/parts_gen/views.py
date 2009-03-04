from django.http import HttpResponse
from django.shortcuts import render_to_response
from fritzing.parts_gen import forms as myforms
import tempfile, zipfile
from django.core.servers.basehttp import FileWrapper
from fritzing.parts_gen.utils import DEST_FOLDER, get_params_def, script_config_from_form, add_folder_to_zipfile


AVAIL_SCRIPTS = [
    {'value':'resistor','label':'Create a resistor'}
]

def choose(request):
    return render_to_response('parts_gen/choose.html', {'scripts_list': AVAIL_SCRIPTS})


def form(request): 
    script_id = request.POST['script_id']
    obj = get_params_def(script_id)
    form = myforms.DynamicForm()
    form.load_fields(obj,script_id)
    return render_to_response('parts_gen/form.html', {'form': form})


def send_zipfile(script_id,config):
    """                                                                         
    Create a ZIP file on disk and transmit it in chunks of 8KB,                 
    without loading the whole file into memory.
    (http://www.djangosnippets.org/snippets/365/)                                 
    """

    temp = tempfile.TemporaryFile()
    archive = zipfile.ZipFile(temp, 'w', zipfile.ZIP_DEFLATED)
    add_folder_to_zipfile(DEST_FOLDER, archive)
    
    archive.close()
    wrapper = FileWrapper(temp)
    response = HttpResponse(wrapper, content_type='application/zip')
    response['Content-Disposition'] = 'attachment; filename=gen_part.fzpz'
    response['Content-Length'] = temp.tell()
    temp.seek(0)
    
    return response


def generate(request):
    if request.method == 'POST': # If the form has been submitted...
        form = myforms.DynamicForm(request.POST) # A form bound to the POST data, not working with the dynamic form
        if form.is_valid(): # All validation rules pass (always true for the dynamic form)
            config, script_id = script_config_from_form(request.POST)
            #print 'script id = ',script_id
            #print 'cfg = ',config
            return send_zipfile(script_id,config)
        else:
            return render_to_response('parts_gen/form.html', {'form': form})
    else:
        return HttpResponse('now form posted')
    
