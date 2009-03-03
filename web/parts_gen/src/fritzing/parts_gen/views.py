from django.http import HttpResponse
from django.shortcuts import render_to_response
from fritzing.parts_gen import forms as myforms
import os, tempfile, zipfile
from django.core.servers.basehttp import FileWrapper


AVAIL_SCRIPTS = [
    {'value':'resistor','label':'Create a resistor'}
]

def choose(request):
    return render_to_response('parts_gen/choose.html', {'scripts_list': AVAIL_SCRIPTS})

def get_params_def(script_id):
    obj = {
        'resistance' : {
            'label': 'Choose the resistance',
            'type' : 'int',
            'range_min' : 1
        },
        'unit' : {
            'label': 'Choose the unit',
            'type' : 'enum',
            'options' : [('kv','kl'),('mv','ml')]
        }
    }
    return obj

def form(request): 
    script_id = request.POST['script_id']
    obj = get_params_def(script_id)
    form = myforms.DynamicForm()
    form.load_fields(obj,script_id)
    return render_to_response('parts_gen/form.html', {'form': form})

def script_config_from_form(data):
    script_id = ''
    config = dict()
    for k in data.keys():
        if k == 'script_id':
            script_id = data[k]
        else:
            config[k] = data[k]  
    return config, script_id

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
    
DEST_FOLDER = '/home/merun/workspace/fritzing/web/parts_gen/parts/'

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

# this two have to be the same, as the one defined for the bundling
# functionalty in the desktop app
ZIP_PART = 'part.'
ZIP_SVG  = 'svg.'

def add_folder_to_zipfile(dirpath, archive):
    for filename in os.listdir(dirpath):
        #archive.write(filename, os.path.abspath(filename).replace('/','.'))
        abspath = dirpath+filename
        if os.path.isdir(abspath):
            add_folder_to_zipfile(abspath+"/", archive)
        else:
            #print os.path.abspath(filename).replace('/','.')
            prefix = ZIP_SVG if abspath.endswith('.svg') else ZIP_PART 
            despfilename = prefix+abspath.replace(DEST_FOLDER,'').replace('svg/','').replace('/','.')
            archive.write(abspath, despfilename)
