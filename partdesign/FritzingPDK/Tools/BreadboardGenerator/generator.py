#!/usr/bin/env python
# encoding: utf-8
"""
generator.py

Created by Dirk van Oosterbosch on 2008-01-08.
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


This script generates a partdescription.xml file for Breadboards.

Run it in the commandline with this:
 python generator.py > partdescription.xml
"""

import sys
import os
from Cheetah.Template import Template

sL = {}
boardLayout = []
rowNets = []
lineNets = []
rowProps = {}
lineProps = {}

def setup():
	# Edit these variables below to generate different breadboards
	#
	# (sL = searchList, a dict to use with the template)
	sL['species'] = "Breadboard"
	sL['title'] = "Breadboard"
	sL['author'] = "Your Name"
	sL['author_url'] = "http://fritzing.org/author/yourname"
	
	# Most breadboards have rows and lines.
	# Rows are 5 holes verticaly
	# Lines are power/ground lines running horizontaly
	
	# Rows 
	rowHeight = 5
	rowsInterBlockSpacing = 2
	rowProps['startsAtXWithNumber'] = -1
	rowProps['startsAtYWithLetter'] = 'a'
	rowProps['segmentLength'] = 64
	rowProps['interSegmentSpacing'] = 2
	rowProps['numberOfSegments'] = 1	
	
	# Lines
	numberOfLinesPerSet = 2
	interRowsLinesSpacing = 2
	lineProps['startAtX'] = 2
	lineProps['subSegmentLength'] = 5
	lineProps['interSubSegmentSpacing'] = 1
	lineProps['segmentLength'] = 5 # measured in subSegments
	lineProps['interSegmentSpacing'] = 2
	lineProps['numberOfSegments'] = 2
	lineProps['endsAtYWithLetter'] = 'z'
	
	# After setting the properties of rows and lines,
	# we compose the total breadboard layout by stacking up
	# lines, rows and spaces.
	
	# Layout
	boardLayout.append( ("lines", numberOfLinesPerSet) )
	boardLayout.append( ("spacing", interRowsLinesSpacing) )
	boardLayout.append( ("rows", rowHeight) )
	boardLayout.append( ("spacing", rowsInterBlockSpacing) )
	boardLayout.append( ("rows", rowHeight) )
	boardLayout.append( ("spacing", interRowsLinesSpacing) )
	boardLayout.append( ("lines", numberOfLinesPerSet) )
	
	return

def displayUsage():
	print """Fritzing Breadboard generator
version 1.1

This script generates the partdescription.xml code for a breadboard part folder.

Usage:
	Run it in the commandline and pipe the result to a file:
	python generator.py run > partdescription.xml

Arguments:
	run
		Use this to run the script. Without this argument, this help text is displayed.

If you want to change the type of breadboard which this script generates
(the main reason for this script to be in the Fritzing PDK), you can easily
edit the python file. The setup() method defines most of the parameters of
the breadboard. Probably changing values here is enough for you to make the
exact breadboard you like.
NOTE: this script doesn't generate image files for the breadboard. You'll
have to make them yourself with your design tools. This script just helps
you creating a consistent partdescription.xml file with all the necessary
connections.

For more information on how to create parts and how to use them in Fritzing,
please read the online documentation:
http://fritzing.org/learning/parts
"""

def calculateTotal(typeToCount, boardLayout):
	count = 0
	for item in boardLayout:
		if item[0] == typeToCount:
			count += item[1]
	return count



def getTemplatefile(fileName):
	thisScriptPath = sys.argv[0]
	thisScriptFolder = os.path.split(thisScriptPath)[0]
	expectedTemplate = os.path.join(thisScriptFolder, fileName)
	if os.path.exists(expectedTemplate):
		return expectedTemplate
	else:
		print "Script Package Incomplete: Couldn't find the svg template file."
		sys.exit()


def main():
	if len(sys.argv) >= 2 and sys.argv[1] == 'run':
		setup()
		# Check if we can identify all y positions with a single letter
		numberOfRowCols = calculateTotal('rows', boardLayout)
		numberOfLines = calculateTotal('lines', boardLayout)
		if (numberOfRowCols + numberOfLines) > 26:
			print "TO MUCH LINES OR ROWS. The script does not know how to name them right. Aborting generation of part"
			sys.exit()
	
		# Make Nets
		incrementalY = 0
		incrLetterY = rowProps['startsAtYWithLetter']
		incrLineYLetterOrd = ord(lineProps['endsAtYWithLetter']) - (numberOfLines - 1)
		for item in boardLayout:
			if item[0] == 'rows':
				for positionX in range(rowProps['segmentLength']):
					net = []
					for posY in range(item[1]):
						dot = {}
						dot['x'] = positionX
						dot['y'] = incrementalY + posY
						charY = chr(ord(incrLetterY) + posY)
						dot['id'] = charY + str(positionX + rowProps['startsAtXWithNumber'])
						net.append(dot)
					rowNets.append(net)
				incrLetterY = chr(ord(incrLetterY) + item[1])
			elif item[0] == 'lines':
				for line in range(item[1]):
					incrementalX = 0
					for segment in range(lineProps['numberOfSegments']):
						net = []
						subIncrX = 0
						for subSegment in range(lineProps['segmentLength']):
							for posX in range(lineProps['subSegmentLength']):
								dot = {}
								dot['x'] = lineProps['startAtX'] + posX + subIncrX + incrementalX
								dot['y'] = incrementalY + line
								charY = chr(incrLineYLetterOrd + line)
								dot['id'] = charY + str(dot['x'] + rowProps['startsAtXWithNumber'])
								net.append(dot)
							subIncrX += lineProps['subSegmentLength'] + lineProps['interSubSegmentSpacing']
						lineNets.append(net)
						incrementalX += subIncrX + lineProps['interSegmentSpacing'] - 1
				incrLineYLetterOrd += line + 1
			incrementalY += item[1]
	
		# Add these nets to the searchList
		sL['rows'] = rowNets
		sL['lines'] = lineNets
	
		t = Template(file=getTemplatefile("breadboard.tmpl"), searchList = [sL])
		print t
	else:
		displayUsage()


if __name__ == '__main__':
	main()
