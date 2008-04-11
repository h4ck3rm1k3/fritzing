#!/usr/bin/env python
# encoding: utf-8
"""
generator.py

version 1.1

Created by Dirk van Oosterbosch on 2008-01-21, for Fritzing.
Copyright (c) 2008 Fachhochschule Potsdam. GNU GPL 2.0

This code is published under the GNU General Public License. It grants to you the four following freedoms:
1. The freedom to run the program for any purpose.
2. The freedom to study how the program works and adapt it to your needs.
3. The freedom to redistribute copies so you can help your neighbor.
4. The freedom to improve the program and release your improvements to the public, so that the whole community benefits.

You may exercise the freedoms specified here provided that you comply with the express conditions of this license. The principal conditions are:

You must conspicuously and appropriately publish on each copy distributed an appropriate copyright notice and disclaimer of warranty and keep intact all the notices that refer to this License and to the absence of any warranty; and give any other recipients of the Program a copy of the GNU General Public License along with the Program. Any translation of the GNU General Public License must be accompanied by the GNU General Public License.

If you modify your copy or copies of the program or any portion of it, or develop a program based upon it, you may distribute the resulting work provided you do so under the GNU General Public License. Any translation of the GNU General Public License must be accompanied by the GNU General Public License.

If you copy or distribute the program, you must accompany it with the complete corresponding machine-readable source code or with a written offer, valid for at least three years, to furnish the complete corresponding machine-readable source code.



This script generates images and a partdescription.xml file for IC Packages.

Run it in the commandline with this:
 python generator.py [-f] Title [numberOfPins [ "text on chip" [wideOrNarrow]]]

where
	Title is the title of the IC Package (e.g. 555Timer)
		A folder named Title will be created and all images and xml will be created therein.
	-f flag is the flag to overwrite or not.
		If a folder with the name Title did already exist and the -f flag is not used
		the script will exit.
	numberOfPins, is the number of pins. Must be an even number, minimal 6. Default is 6
	"text on chip" is the text on the chip. If more than one word, use quotes (").
	wideOrNarrow is whether the dil package will be a wide or narrow package
	 	(i.e. bridging 3 or 6 pins). Default is narrow. ((In fact narrow is the only one working, yet))
"""

import sys
import os
import re
from distutils.file_util import copy_file
from Cheetah.Template import Template
import Image

ARGUMENT_DEBUG = False
sL = {}
DEFAULT_N_PINS = 6
DEFAULT_NAME_OF_PIN = "Name This Pin"
BATIK_FOLDER = 'batik-1.7'
RASTERIZER_JAR = 'batik-rasterizer.jar'
SVG_TEMPLATE_FILE = 'dilpackage_svg.tmpl'
SVG_ICON_TEMPLATE_FILE = 'dilpackage_with_pins_svg.tmpl'
PARTDESC_TEMPLATE_FILE = 'dilpackagedescription.tmpl'
temporarySvgFileName = 'dilpackage_temp.svg'
temporaryPngIconFileName = 'fullicon.png'
availableNarrowPackages = [6, 8, 14, 16, 18, 20, 22, 24, 28, 32]
NARROW_FOOTPRINTS = "narrowFootprints.lbr"

def setup():
	# sL['species'] = packageName
	sL['genus'] = "IC.Logic"
	# sL['title'] = packageName
	sL['description'] = "Please describe your part here"
	sL['label'] = "IC"
	sL['reference'] = "http://www.example.com/url_to_online_reference"
	sL['author'] = "Add Your Name Here"
	sL['author_url'] = "http://fritzing.org/author/yourname"
	return

def displayUsage():
	print """Fritzing DIL Package IC generator
version 1.0

This script creates Fritzing DIL-Package IC parts. It generates the images and the
partdescription.xml file for those parts in a part folder.

Usage:
 python generator.py [-f] TITLE [numberOfPins [ "text on chip" [wideOrNarrow]]]

Arguments:
	-f
		the flag to overwrite or not.
		If a folder with the name TITLE did already exist and the -f flag is
		not used the script will exit.
	TITLE
		the title of the IC Package (e.g. 555Timer)
		A folder named TITLE will be created and all images and xml will be created therein.
		
	numberOfPins
		the number of pins. Must be an even number, minimal 6.
		If omitted 6 will be used as number of pins.
		
	"text on chip"
		the text which is printed on the chip
		If more than one word is used, it needs to be in quotes. (")
		
	wideOrNarrow
	 	argument indicating is whether to create a wide or narrow package.
		Has to be 'wide' or 'narrow'.
		Narrow means bridging 3 pins. Wide means bridging 6 pins.
		If omitted 'narrow' will be used. (In fact, narrow is the only one working, yet, sorry)


For more information on how to create parts and how to use them in Fritzing,
please read the online documentation:
http://fritzing.org/learning/parts
"""

def getBatikFolder():
	thisScriptFolder = sys.path[0]
	expectedBatik = os.path.join(thisScriptFolder, BATIK_FOLDER)
	if os.path.exists(expectedBatik) and os.path.isdir(expectedBatik):
		return expectedBatik
	else:
		print "\nError: Couldn't locate the batik folder. Make sure you have it at the same location as the generator.py script"
		sys.exit()

def getBatikRasterizer():
	batikFolder = getBatikFolder()
	expectedJar = os.path.join(batikFolder, RASTERIZER_JAR)
	if os.path.exists(expectedJar):
		return expectedJar
	else:
		print "\nError: Couldn't find a Batik Rasterizer .jar file in the Batik folder"
		sys.exit()

def getFootprintFile(fileName):
	thisScriptFolder = sys.path[0]
	expectedFootprint = os.path.join(thisScriptFolder, fileName)
	if os.path.exists(expectedFootprint):
		return expectedFootprint
	else:
		print "\nError: Script Package Incomplete: Couldn't find the footprint file."
		sys.exit()

def getTemplatefile(fileName):
	thisScriptFolder = sys.path[0]
	expectedTemplate = os.path.join(thisScriptFolder, fileName)
	if os.path.exists(expectedTemplate):
		return expectedTemplate
	else:
		print "\nError: Script Package Incomplete: Couldn't find the svg template file."
		sys.exit()

def main():
	numberOfPins = 0
	forceFlag = False
	textOnChip = ''
	wideOrNarrow = 'narrow'
	if len(sys.argv) >= 2:
		# Getting Arguments
		#
		# Probably this code could better be rewritten, using getopt(),
		# like with this Cookbook example
		# http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/302262
		# but I was too lazy to do that.
		# If anybody cares, please do so. :)
		# 
		# Get any flags if there are any used:
		arglist = sys.argv
		for arg in arglist[1:]:
			if arg[:1] == "-":
				# This is a flag. Is this the -f flag?
				if arg == '-f':
					forceFlag = True
				# Any other flags are discarded
				sys.argv.remove(arg)
		# Get the package name
		packageName = sys.argv[1]
		if len(sys.argv) >= 3:
			# Get the number of pins
			numberOfPins = sys.argv[2]
		if len(sys.argv) >= 4:
			# Get the text on the chip
			textOnChip = sys.argv[3]
		if len(sys.argv) >= 5:
			# Get the narrow or wideOrNarrow
			wideOrNarrow = sys.argv[4]
 		# Calculate the numberOfPins
		if numberOfPins:
			pinsPattern = re.compile(r'\d+')
			if pinsPattern.match(numberOfPins):
				numberOfPins = int(numberOfPins)
				if numberOfPins % 2:
					numberOfPins += 1
			else:
				numberOfPins = DEFAULT_N_PINS
		else:
			numberOfPins = DEFAULT_N_PINS
		# Print Statments to check the parameters
		print "packageName : %s" % packageName
		print "numberOfPins: %s" % numberOfPins
		print "forceFlag   : %s" % forceFlag
		print "textOnChip  : %s" % textOnChip
		print "wideOrNarrow: %s" % wideOrNarrow
		
		# Test if the number of pins is an existing DIL Package
		if (wideOrNarrow == 'narrow'):
			if not numberOfPins in availableNarrowPackages:
				print "\nWarning: Number of pins is wrong for DIL Package:\n\tDIL packages with %d pins do not exits in the %s package.\n\tThe script will continue, but the resulting part might not be able to create a PCB with." % (numberOfPins, wideOrNarrow)
	
		
		if (ARGUMENT_DEBUG):
			# Exit For debugging:
			sys.exit()
		
		# Check if the directory did not already exist
		try:
			os.mkdir(packageName)
		except OSError, error:
			if not forceFlag:
				print "\nError: The directory %s already exists. Script aborted. Use -f to override" % (packageName)
				sys.exit()
		try:
			os.chdir(packageName)
		except OSError, e:
			raise e
			sys.exit()
		setup()
		# Create temporary svg file in this directory
		# Only if we have a rasterizer to do something with the .svg's
		if getBatikRasterizer():
			# Find the svg template
			if getTemplatefile(SVG_TEMPLATE_FILE):
				sL['width'] = 10 * (numberOfPins / 2)
				sL['nMiddlePins'] = (numberOfPins / 2) - 2 
				sL['textOnChip'] = textOnChip
				t = Template(file=getTemplatefile(SVG_TEMPLATE_FILE), searchList = [sL])
				temporarySvgFile = open(temporarySvgFileName, 'w')
				try:
					temporarySvgFile.write(t.respond())
				finally:
					temporarySvgFile.close()
				# create scaled images from this svg
				zoomLevels = [1,2,4]
				for level in zoomLevels:
					print "Rasterizing image for zoom-level %d00%% ..." % (level)
					outputFile = "part%d.png" % (level * 100)
					outputWidth = (10 * (numberOfPins / 2)) * level
					os.system("java -jar \"%s\" -dpi 72 -w %d -d \"%s\" \"%s\"" % (getBatikRasterizer(), outputWidth, outputFile, temporarySvgFileName))
				# 
				# Create an extra image on 100% with pins
				t = Template(file=getTemplatefile(SVG_ICON_TEMPLATE_FILE), searchList = [sL])
				temporarySvgFile = open(temporarySvgFileName, 'w')
				try:
					temporarySvgFile.write(t.respond())
				finally:
					temporarySvgFile.close()
				# create 100% image from this svg
				print "Rasterizing image for icon (100%% with legs) ..."
				outputFile = temporaryPngIconFileName
				# outputWidth = 10 * (numberOfPins / 2)
				# The use of the -h 35 is a hack, because the svg is not perfect ...
				os.system("java -jar \"%s\" -dpi 72 -h 35 -d \"%s\" \"%s\"" % (getBatikRasterizer(), outputFile, temporarySvgFileName))
				# Create the 32x32 and the 16x16 pixel icon through PIL
				try:
					orgImg = Image.open(outputFile)
					largeIcon = Image.new('RGBA', (32,32))
					smallIcon = Image.new('RGBA', (16,16))
					largeIcon.paste(orgImg, (2,-2))
					smallIcon.paste(orgImg, (1,-3))
					largeIcon.save('iconLargeTransp.png', "PNG")
					largeIcon.save('iconLarge.gif', "GIF") # Still neccessary for now
					smallIcon.save('iconSmallTransp.png', "PNG")
					smallIcon.save('iconSmall.gif', "GIF") # Still neccessary for now
					# The gif images PIL creates are very ugly. But it's only necessary for temporarily
				except Exception, e:
					print e
					print "\nError: Python Image Library couldn't be used. Make sure you installed it correctly"
				#
				# Create the partdescription.xml file
				#
				sL['species'] = packageName
				sL['title'] = packageName
				sL['hiwidth'] = 254 * (numberOfPins / 2)
				if (wideOrNarrow == 'narrow'):
					sL['hiheight'] = 724
				else:
					sL['hiheight'] = 724
				# Pick the right footprint
				theFootprintFile = ""
				if (wideOrNarrow == 'narrow'):
					if numberOfPins in availableNarrowPackages:
						sL['footprint'] = "%s/DIL%02d" % (NARROW_FOOTPRINTS, numberOfPins)
						theFootprintFile = NARROW_FOOTPRINTS
					else:
						sL['footprint'] = "NO_FOOTPRINT_AVAILALBLE.lbr"
				else:
					sL['footprint'] = "WIDE_DIL_PACKAGE_NOT_IMPLEMENTED_YET.lbr"
				# Copy the footprint file into the designated part folder
				if getFootprintFile(theFootprintFile):
					print "Copying the correct footprint in the part folder ..."
					copy_file(getFootprintFile(theFootprintFile), theFootprintFile)
				# Make the legs
				legList = []
				for num in range(numberOfPins):
					aLeg = {}
					aLeg['id'] = num + 1
					aLeg['name'] = DEFAULT_NAME_OF_PIN
					if num < (numberOfPins / 2):
						aLeg['startxhi'] = num * 254
						aLeg['endx'] = num
						aLeg['startyhi'] = 683
						aLeg['endy'] = 3
					else:
						aLeg['startxhi'] = (numberOfPins - (num + 1)) * 254
						aLeg['endx'] = numberOfPins - (num + 1)
						aLeg['startyhi'] = 69
						aLeg['endy'] = 0
					legList.append(aLeg) 
				sL['legs'] = legList
				# Render the template into the partdescription.xml file
				t = Template(file=getTemplatefile(PARTDESC_TEMPLATE_FILE), searchList = [sL])
				partDescFile = open('partdescription.xml', 'w')
				try:
					partDescFile.write(t.respond())
				finally:
					partDescFile.close()
				
				# remove the temporary svg and icon png files again
				os.remove(temporaryPngIconFileName)
				os.remove(temporarySvgFileName)
				print "Ready"
				print """----------------------------------------------------------------
				
				
Now your part folder is created, please edit the partdescription.xml therein.
Specifically edit the part identifier: Genus and Species
And add the specific names to the pins of your chip, so you can easilily identify them."""
	else:
		displayUsage()
		sys.exit()


if __name__ == '__main__':
	main()
