import os
from dispatcher import GEN_FILES_FOLDER


# this two have to be the same, as the ones defined for the bundling
# functionalty in the desktop app (mainwindow.cpp)
ZIP_PART = 'part.'
ZIP_SVG  = 'svg.'

def add_folder_to_zipfile(dirpath, archive):
    for filename in os.listdir(dirpath):
        if filename.startswith("."):
            continue
        else:
            abspath = dirpath+filename
            if os.path.isdir(abspath):
                add_folder_to_zipfile(abspath+"/", archive)
            else:
                prefix = ZIP_SVG if abspath.endswith('.svg') else ZIP_PART 
                despfilename = prefix+abspath.replace(GEN_FILES_FOLDER,'').replace('svg/','').replace('/','.')
                archive.write(abspath, despfilename)


def script_config_from_form(data):
    script_id = ''
    config = dict()
    for k in data.keys():
        if k == 'script_id':
            script_id = data[k]
        else:
            config[k] = data[k]  
    return config, script_id
