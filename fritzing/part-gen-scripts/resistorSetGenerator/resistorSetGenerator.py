#!/usr/bin/env python
# encoding: utf-8
"""
/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fritzing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************

$Revision: 1595 $:
$Author: dirk@fritzing.org $:
$Date: 2008-11-21 11:04:36 +0100 (Fri, 21 Nov 2008) $

********************************************************************/

resistorSetGenerator.py

"""

import sys, getopt, ConfigParser, os, uuid
from datetime import date
from Cheetah.Template import Template

import valuesAndColors_forResistors

help_message = """
Usage:
	resistorSetGenerator.py -s [Resistor Set] -o [Output Dir]
	
	Resistor Set - the named set of standard resistors that should be generated
					Possible values are: E3, E6, E12, E24 for the E3Set, E6Set etc.
					
	Output Dir - the location where the output files are written
"""


class Usage(Exception):
	def __init__(self, msg):
		self.msg = msg

def makeUUID():
	"creates an 8 character hex UUID"
	return str(uuid.uuid1())

def makeDate():
	"creates a date formatted as YYYY-MM-DD"
	return date.today().isoformat()

def getUserName():
	"gets the full user name if available"
	try:
		import pwd
		return pwd.getpwuid(os.getuid())[4]
	except:
		return os.getlogin()

def getTemplatefile(fileName):
	thisScriptPath = sys.argv[0]
	thisScriptFolder = os.path.split(thisScriptPath)[0]
	expectedTemplate = os.path.join(thisScriptFolder, fileName)
	if os.path.exists(expectedTemplate):
		return expectedTemplate
	else:
		print >> sys.stderr, "Script Package Incomplete: Couldn't find the svg template file: %s" % (fileName)
		sys.exit(2)

def getConfigFile(fileName):
	thisScriptPath = sys.argv[0]
	thisScriptFolder = os.path.split(thisScriptPath)[0]
	expectedTemplate = os.path.join(thisScriptFolder, fileName)
	if os.path.exists(expectedTemplate):
		return expectedTemplate
	else:
		print >> sys.stderr, "Script Package Incomplete: Couldn't load the config file: %s" % (fileName)
		sys.exit(2)

def writeOutFileInSubDirectoryWithData(outputDir,  subDirectory, fileName, data):
	outputDir = os.path.join(outputDir, subDirectory)
	if not os.path.exists(outputDir):
		os.makedirs(outputDir)
	outfile = open(os.path.join(outputDir, fileName), "w")
	outfile.write(str(data))
	outfile.close()

def main(argv=None):
	if argv is None:
		argv = sys.argv
	try:
		try:
			opts, args = getopt.getopt(argv[1:], "ho:s:", ["help", "output=", "set="])
		except getopt.error, msg:
			raise Usage(msg)
		
		output = "."	
		verbose = False
		namedSet = None
	
		# option processing
		for option, value in opts:
			if option in ("-h", "--help"):
				raise Usage(help_message)
			if option in ("-o", "--output"):
				output = value
			if option in ("-s", "--set"):
				namedSet = value
		
		if(not(namedSet) or not(output)):
			print >> sys.stderr, "No Resistor Set or Output Dir specified"
			raise Usage(Exception)
		
		config = ConfigParser.ConfigParser()
		config.readfp(open(getConfigFile('defaults.cfg')))
		
		resistorSet = valuesAndColors_forResistors.resistorSetForSetName(namedSet)
		print "number of resistors in set: %d" % (len(resistorSet))
		resistorBin = []
		for r in resistorSet:
			sL = {}
			metaData = {}
			resistorInBin = {}
			metaData['moduleID'] = makeUUID()
			metaData['date'] = makeDate()
			metaData['author'] = getUserName()
			sL['metaData'] = metaData
			resistanceString = valuesAndColors_forResistors.valueStringFromValue(r)
			sL['resistance'] = resistanceString
			# sL['taxonomy'] = "discreteParts.resistor.%s" % (resistanceString)
			# The resistance with the dot notation in the taxonomy might be a problem, so we'd better do:
			sL['taxonomy'] = "discreteParts.resistor.%d" % (r)
			sL['colorBands'] = valuesAndColors_forResistors.hexColorsForResistorValue(r)
			# Breadboard svg
			svg = Template(file=getTemplatefile("resistor_breadboard_svg.tmpl"), searchList = [sL])
			writeOutFileInSubDirectoryWithData(output, "breadboard", "resistor_%s.svg" % (resistanceString), svg)
			# Icon svg
			svg = Template(file=getTemplatefile("resistor_icon_svg.tmpl"), searchList = [sL])
			writeOutFileInSubDirectoryWithData(output, "icon", "resistor_icon_%s.svg" % (resistanceString), svg)
			# Partfile fz
			# TODO Part should be a .fzp file
			fzfilename = "resistor_%s.fz" % valuesAndColors_forResistors.valueStringWithoutDots(r)
			fz = Template(file=getTemplatefile("resistor_fz.tmpl"), searchList = [sL])
			writeOutFileInSubDirectoryWithData(output, "fz_files", fzfilename, fz)
			# For in the bin
			resistorInBin['moduleID'] = metaData['moduleID']
			resistorInBin['filename'] = fzfilename
			resistorBin.append(resistorInBin)
		# Bin file
		if (config.getboolean('OutputBin', 'outputBin')):
			sL = {}
			sL['title'] = "%s Resistor Set" % (namedSet)
			partsDir = config.get('OutputBin', 'partsDir')
			for r in resistorBin:
				r['filepath'] = os.path.join(partsDir, r['filename'])
			sL['resistors'] = resistorBin
			fz = Template(file=getTemplatefile("bin_fz.tmpl"), searchList = [sL])
			# TODO Bin should be a .fzb file
			writeOutFileInSubDirectoryWithData(output, "bin", "%sBin.fz" % (namedSet), fz)
	
	except Usage, err:
		print >> sys.stderr, sys.argv[0].split("/")[-1] + ": " + str(err.msg)
		print >> sys.stderr, "\t for help use --help"
		return 2


if __name__ == "__main__":
	sys.exit(main())
	