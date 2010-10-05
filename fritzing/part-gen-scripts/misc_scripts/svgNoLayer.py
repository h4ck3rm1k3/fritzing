# usage:
#    svgNoLayer.py -d <directory> 
#    
#    <directory> is a folder, with subfolders, containing .svg files.  In each svg file in the directory or its children
#    look for id='[layer]' where layer is the set of all layers in fritzing

import getopt, sys, os, re
    
def usage():
    print """
usage:
    svgNoLayer.py -d [directory]
    
    <directory> is a folder, with subfolders, containing .svg files.  In each svg file in the directory or its children 
    look for id='[layer]' where layer is the set of all layers in fritzing
    """
layers = ["icon","breadboardbreadboard", "breadboard", "breadboardWire", "breadboardLabel", "breadboardNote", "breadboardRuler", "schematic", "schematicWire","schematicTrace","schematicLabel", "schematicRuler", "board", "ratsnest", "silkscreen", "silkscreenLabel", "groundplane", "copper0", "copper0trace", "groundplane1", "copper1", "copper1trace", "silkscreen0", "silkscreen0Label", "soldermask",  "outline",  "keepout", "partimage", "pcbNote", "pcbRuler"]

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hd:", ["help", "directory"])
    except getopt.GetoptError, err:
        # print help information and exit:
        print str(err) # will print something like "option -a not recognized"
        usage()
        sys.exit(2)
    outputDir = None
    
    for o, a in opts:
        #print o
        #print a
        if o in ("-d", "--directory"):
            outputDir = a
        elif o in ("-h", "--help"):
            usage()
            sys.exit(2)
        else:
            assert False, "unhandled option"
    
    if(not(outputDir)):
        usage()
        sys.exit(2)
        
 
    for root, dirs, files in os.walk(outputDir, topdown=False):
        for filename in files:
            if (filename.endswith(".svg")):  
                infile = open(os.path.join(root, filename), "r")
                svg = infile.read()
                infile.close()
                match = None
                m2 = None
                for layer in layers:
                    match = re.search('id=[\'\"]' + layer, svg)
                    if (match != None):
                        break
                        
                if match == None:
                    print "{0} {1}".format(os.path.join(root, filename), "")
                else:
                    if lookForSvgId(svg):
                        print "{0} {1}".format(os.path.join(root, filename), "svg has id")
                        

def lookForSvgId(svg):
    for layer in layers:
        match = re.search('<svg(.*?)id=[\'\"]' + layer  , svg)
        if (match != None):
            if (match.group(1).find(">") < 0):
                return 1
            

    return 0
    
    
if __name__ == "__main__":
    main()

