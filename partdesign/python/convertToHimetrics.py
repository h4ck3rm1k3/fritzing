#!/usr/bin/env python
# encoding: utf-8
"""
convertToHimetrics.py

Created by Dirk van Oosterbosch on 2008-01-10.
Copyright (c) 2008 Fachhochschule Potsdam. GPL
"""

import sys
import os
import re

MULTIPLY_FACTOR = 25.4
HIGH400_FACTOR = 6.35

def createTagFromSizeString(kind, sizeString):
	pattern = re.compile(r'(-?\d+)\s*x\s*(-?\d+)')
	m = pattern.match(sizeString)
	if m:
		val_x = int(m.groups()[0])
		val_y = int(m.groups()[1])
		if (kind == 'bounds'):
			high_w = round(val_x * MULTIPLY_FACTOR)
			high_h = round(val_y * MULTIPLY_FACTOR)
			return '\t<bounds width="%d" height="%d" units="himetric"/> <!-- %dx%d pixels -->\n' % (high_w, high_h, val_x, val_y)
		elif (kind == 'leg'):
			high_w = round(val_x * HIGH400_FACTOR)
			high_h = round(val_y * HIGH400_FACTOR)
			return '\t<point x="%d" y="%d" units="himetric"/> <!-- %dx%d 400%% view pixels -->\n' % (high_w, high_h, val_x, val_y)
		elif (kind == 'hibounds'):
			high_w = round(val_x * HIGH400_FACTOR)
			high_h = round(val_y * HIGH400_FACTOR)
			return '\t<bounds width="%d" height="%d" units="himetric"/> <!-- %dx%d 400%% view pixels -->\n' % (high_w, high_h, val_x, val_y)
		elif (kind == 'gridoffset4'):
			high_w = round(val_x * HIGH400_FACTOR)
			high_h = round(val_y * HIGH400_FACTOR)
			return '\t<gridOffset x="%d" y="%d" units="himetric"/> <!-- %dx%d 400%% view pixels -->\n' % (high_w, high_h, val_x, val_y)
	else:
		return "Error: pattern did not match 'n x n'"

def main():
	if len(sys.argv) >= 3:
		stringToConvert = " ".join(sys.argv[2:])
		conversionKind = sys.argv[1]
		if conversionKind == 'offset': conversionKind = 'gridoffset4'
		print createTagFromSizeString(conversionKind, stringToConvert)
	else:
		print "Error: not enough arguments"

if __name__ == '__main__':
	main()

