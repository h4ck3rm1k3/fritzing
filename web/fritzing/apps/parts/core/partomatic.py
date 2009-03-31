# generic wrapper for cheetah templates
# usage:
#    partomatic.py -c <configfile> -o <output dir> -t <template file>
#    
#    where 
#    <config file> is the name of the file containing a list of variables and values
#                   specified on separate lines separated by colons.  
#                   ex:
#                      [file1.fz]
#                      value: 12345
#                      color1: #C1D1D1
#
#   <output dir> is the location where the output files are written
#
#   <template file> is the Cheetah template used to generate the output

import getopt, sys, ConfigParser, uuid, os
from datetime import date
from Cheetah.Template import Template
from fritzing.apps.parts.core.utils import escape_to_file_name
from fritzing.apps.parts.core.confs import SCRIPT_CONFS


_HELPER = { # helper class to name the uuid to append to the files
    ''           : 'part_unique',
    'icon'       : 'icon_unique',
    'breadboard' : 'bread_unique',
    'schematic'  : 'schem_unique',
    'pcb'        : 'pcb_unique'
}

def usage():
    print """
usage:
    partomatic.py -c [Config File] -o [Output Dir] -t [Template File] -s [file suffix]
    
    Config File - the name of the file containing a list of variables and values
                   specified on separate lines separated by colons.  
                   ex:
                      [file1]
                      value: 12345
                      color1: #C1D1D1

   Output Dir - the location where the output files are written

   Template File - the Cheetah template used to generate the output
    """
    
def makeUUID():
    "creates an 8 character hex UUID"
    #print "making new uuid"
    return str(uuid.uuid1())
    
def makeDate():
    "creates a date formatted as YYYY-MM-DD"
    return date.today().isoformat()
    
def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "ho:c:t:s:", ["help", "output="])
    except getopt.GetoptError, err:
        # print help information and exit:
        print str(err) # will print something like "option -a not recognized"
        usage()
        sys.exit(2)
    output = None
    verbose = False
    configFile = None
    outputDir = None
    templateFile = None
    suffix = ""
    
    for o, a in opts:
        if o in ("-c", "--config"):
            configFile = a
        elif o in ("-h", "--help"):
            usage()
            sys.exit(2)
        elif o in ("-o", "--outdir"):
            outputDir = a
        elif o in ("-t", "--template"):
            templateFile = a
        elif o in ("-s", "--suffix"):
            suffix = a.lstrip(".")
        else:
            assert False, "unhandled option"
    
    if(not(configFile) or not(outputDir) or not(templateFile)):
        usage()
        sys.exit(2)
        
    cfgParser = ConfigParser.ConfigParser()
    
    cfgParser.readfp(open(configFile))
    
    for section in cfgParser.sections():
        nameStub = section + "." + suffix
        cfgDict = {}
        #populate the dictionary
        for cfgItem, cfgValue in cfgParser.items(section):
            if(cfgValue == "$UUID"):
                cfgValue = makeUUID()
            if(cfgValue == "$DATE"):
                cfgValue = makeDate()
            cfgDict[cfgItem] = cfgValue
        render_templates(cfgDict,templateFile,outputDir,nameStub)


def create_output_folder(output_folder_base):
    folders = ["icon","breadboard","schematic","pcb"]
    prefix = output_folder_base+"/"+makeUUID()
    os.mkdir(prefix)
    for folder in folders:
        os.mkdir(prefix+"/"+folder)
    return prefix


def web_generate(script_id, config, output_folder_base):
    script_data = SCRIPT_CONFS[script_id]
    assert script_data
        
    output_dir = create_output_folder(output_folder_base)
    config['uuid'] = makeUUID()
    config['date'] = makeDate()
    
    for uniques in _HELPER.values():
        config[uniques] = makeUUID()
        
    suffix = escape_to_file_name(config[script_data['name_param']])
    
    for folder, template in script_data['templates'].items():
        extension = ('.fzp' if folder == '' else '.svg')
        name_stub = template \
                    + "_"+suffix \
                    + "_"+config[_HELPER[folder]] \
                    + extension
        
        aux = os.path.join(os.path.dirname(__file__),"templates")
        template_aux = os.path.join(aux,template+extension) 
        render_templates(config, template_aux, os.path.join(output_dir,folder), name_stub)
        
    return output_dir, suffix


def render_templates(config,templateFile,outputDir,nameStub):
    outfile = open(os.path.join(outputDir, nameStub), "w")
    page = Template(file=templateFile, searchList=[config])
    outfile.write(str(page))
    outfile.close()

if __name__ == "__main__":
    main()

