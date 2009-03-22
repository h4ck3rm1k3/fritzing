import os

# this two have to be the same, as the ones defined for the bundling
# functionalty in the desktop app (mainwindow.cpp)
ZIP_PART = 'part.'
ZIP_SVG  = 'svg.'

def add_folder_to_zipfile(dirpath, archive, gen_files_folder):
    for filename in os.listdir(dirpath):
        if filename.startswith("."):
            continue
        else:
            abspath = os.path.join(dirpath,filename)
            if os.path.isdir(abspath):
                add_folder_to_zipfile(os.path.join(abspath,""), archive, gen_files_folder)
            else:
                prefix = ZIP_SVG if abspath.endswith('.svg') else ZIP_PART 
                despfilename = prefix+abspath.replace(gen_files_folder,'').replace('svg/','').replace('/','.')
                archive.write(abspath, despfilename.replace("..","."))
            print ""


def script_config_from_form(data):
    script_id = ''
    config = dict()
    for k in data.keys():
        if k == 'script_id':
            script_id = data[k]
        else:
            config[k] = data[k]  
    return config, script_id
