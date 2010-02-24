# usage:
#	fzpzclean.py -f [fzpz file] -d [directory]
#	unzip the given fzpz file into the directory and remove all the guids from the filenames and internal names.
#	maybe eventually check that the name is unique

# lots of borrowing from http://code.activestate.com/recipes/252508-file-unzip/

import getopt, sys, os, os.path, re, zipfile
    
def usage():
    print """
usage:
    fzpzclean.py -f [fzpz file] -d [directory]
    unzip the given fzpz file into the directory and remove all the guids from the filenames and internal names.
    maybe eventually check that the name is unique
    """
    
  	
	   
def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hf:d:", ["help", "file", "directory"])
    except getopt.GetoptError, err:
        # print help information and exit:
        print str(err) # will print something like "option -a not recognized"
        usage()
        sys.exit(2)
    file = None
    dir = None
    
    for o, a in opts:
        #print o
        #print a
        if o in ("-f", "--file"):
            file = a
        elif o in ("-d", "--directory"):
            dir = a
        elif o in ("-h", "--help"):
            usage()
            sys.exit(2)
        else:
            assert False, "unhandled option"
    
    if(not(file)):
        usage()
        sys.exit(2)

    if(not(dir)):
        usage()
        sys.exit(2)

    if not dir.endswith(':') and not os.path.exists(dir):
        os.mkdir(dir)
   
    zf = zipfile.ZipFile(file)

    # create directory structure to house files
    createstructure(file, dir)

    # extract files to directory structure
    for i, name in enumerate(zf.namelist()):
        if not name.endswith('/'):
            outname = re.sub('^svg\.((icon)|(breadboard)|(schematic)|(pcb))\.', '', name, 1)
            outname = re.sub('^part\.', '', outname, 1)
            outname = re.sub('__[0-9a-fA-F]{32}', '', outname)
            outname = re.sub('__[0-9a-fA-F]{27}', '', outname)
            outfile = open(os.path.join(dir, outname), 'wb')
            if name.endswith(".fzp"):
                s = zf.read(name)
                s = re.sub('__[0-9a-fA-F]{32}', '', s)
                s = re.sub('__[0-9a-fA-F]{27}', '', s)
                outfile.write(s)
            else:
                outfile.write(zf.read(name))
            outfile.flush()
            outfile.close()

        
def createstructure(file, dir):
    makedirs(listdirs(file), dir)


def makedirs(directories, basedir):
    """ Create any directories that don't currently exist """
    for dir in directories:
        curdir = os.path.join(basedir, dir)
        if not os.path.exists(curdir):
            os.mkdir(curdir)

def listdirs(file):
    """ Grabs all the directories in the zip structure
    This is necessary to create the structure before trying
    to extract the file to it. """
    zf = zipfile.ZipFile(file)

    dirs = []

    for name in zf.namelist():
        if name.endswith('/'):
            dirs.append(name)

    dirs.sort()
    return dirs


if __name__ == "__main__":
    main()



