#!/usr/bin/env python
# encoding: utf-8
"""
convertToHimetrics.py

Created by Dirk van Oosterbosch on 2008-01-10, for Fritzing.
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

"""

import sys
import os
import re

MULTIPLY_FACTOR = 25.4
HIGH400_FACTOR = 6.35

def displayUsage():
	print """Simple script to convert pixel measurements into highmetric.
For use in the Fritzing partdescription.xml files. It takes a pixel vector e.g. 5x12 and
converts this into a <tag> which can be placed in your partdescription.xml file directly.

Usage:
	python convertToHimetrics.py KIND PIXELS
	
Arguments:
	KIND
		the kind of conversion which is performed and the kind of tag which is returned.
		Possible kinds are:
			bounds
				creates <bounds> tag.
				Use 100%% view pixels as measurement
			offset
				creates <gridOffset> tag.
				Use 400%% view pixels as measurement
			leg
				creates <point> tag which can be used as a child of <connector>.
				Use 400%% view pixels as measurement
			hibounds
				creates <bounds> tag.
				Use 400%% view pixels as measurement
			pixeloffset
				creates <gridOffset> tag.
				Use 100%% view pixels as measurement
			pixelleg
				creates <point> tag which can be used as a child of <connector>.
				Use 100%% view pixels as measurement
			hioffset
				equal to 'offset'
			hileg
				equal to 'leg'
	
	PIXELS
		the size or offset measured in pixels.
		The pattern for this must follow: 'n x n', where
			n
				is the number in pixels.
				It should be an integer, but can also be negative (e.g. -40)

Example:
	You want to create a <gridOffset> tag. In the 400%% view of your drawing application
	you are measuring a 50 x 349 square between the top-left corner of your 400%% part
	and the grid point that should be the grid origin (where x="0" and y="0").
	Then use the following command:
		python convertToHimetrics.py offset 50 x 349
	it returns
		<gridOffset x="318" y="2216" units="himetric"/> <!-- 50x349 400%% view pixels -->	
	which you can paste into your partdescription.xml file. 
	

For more information on how to create parts and how to use them in Fritzing,
please read the online documentation:
http://fritzing.org/learning/parts
"""

def createTagFromSizeString(kind, sizeString):
	pattern = re.compile(r'(-?\d+)\s*[xX]\s*(-?\d+)')
	m = pattern.match(sizeString)
	if m:
		val_x = int(m.groups()[0])
		val_y = int(m.groups()[1])
		# Bounds
		if (kind == 'bounds'):
			high_w = round(val_x * MULTIPLY_FACTOR)
			high_h = round(val_y * MULTIPLY_FACTOR)
			return '\t<bounds width="%d" height="%d" units="himetric"/> <!-- %dx%d pixels -->\n' % (high_w, high_h, val_x, val_y)
		elif (kind == 'hibounds'):
			high_w = round(val_x * HIGH400_FACTOR)
			high_h = round(val_y * HIGH400_FACTOR)
			return '\t<bounds width="%d" height="%d" units="himetric"/> <!-- %dx%d 400%% view pixels -->\n' % (high_w, high_h, val_x, val_y)
		# Leg
		elif (kind == 'pixelleg'):
			high_w = round(val_x * MULTIPLY_FACTOR)
			high_h = round(val_y * MULTIPLY_FACTOR)
			return '\t<point x="%d" y="%d" units="himetric"/> <!-- %dx%d view pixels -->\n' % (high_w, high_h, val_x, val_y)
		elif (kind == 'hileg'):
			high_w = round(val_x * HIGH400_FACTOR)
			high_h = round(val_y * HIGH400_FACTOR)
			return '\t<point x="%d" y="%d" units="himetric"/> <!-- %dx%d 400%% view pixels -->\n' % (high_w, high_h, val_x, val_y)
		# Offset
		elif (kind == 'pixeloffset'):
			high_w = round(val_x * MULTIPLY_FACTOR)
			high_h = round(val_y * MULTIPLY_FACTOR)
			return '\t<gridOffset x="%d" y="%d" units="himetric"/> <!-- %dx%d view pixels -->\n' % (high_w, high_h, val_x, val_y)
		elif (kind == 'hioffset'):
			high_w = round(val_x * HIGH400_FACTOR)
			high_h = round(val_y * HIGH400_FACTOR)
			return '\t<gridOffset x="%d" y="%d" units="himetric"/> <!-- %dx%d 400%% view pixels -->\n' % (high_w, high_h, val_x, val_y)
	else:
		# displayUsage()
		return """Error: PIXELS pattern did not match 'n x n'
		
type python %s 
without any arguments to display the help text.""" % (sys.argv[0])

def main():
	if len(sys.argv) >= 3:
		stringToConvert = " ".join(sys.argv[2:])
		conversionKind = sys.argv[1]
		if conversionKind == 'offset': conversionKind = 'hioffset'
		if conversionKind == 'leg': conversionKind = 'hileg'
		print createTagFromSizeString(conversionKind, stringToConvert)
	else:
		displayUsage()

if __name__ == '__main__':
	main()

