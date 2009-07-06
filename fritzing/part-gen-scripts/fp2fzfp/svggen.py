#this object takes an fzfp (an xml-ized gEDA footprint) and makes
# it into a Fritzing footprint SVG.
import xml.dom.minidom

class SVGGen(xml.dom.minidom.Document):
    def __init__(self, xmlFile):
    
    
    def getSvgFile(self):
    # spits out a string with the generated SVG
        pass
    
    def _drawNode(self, node):
    # renders a node to SVG
        pass
        
    def _drawPin(self, node):
    # render a pin to SVG
        pass
        
    def _drawPad(self, node):
    # render a copper pad to SVG
        pass
        
    def _drawElementLine(self, node):
    # render a silkscreen line
        pass
        
    def _drawElementArc(self, node):
    # render a silkscreen circular arc
        pass
        
    def _drawMark(self, node):
    # render the center point mark
        pass
        
    def minMax(self, x, y, strokeWidth):
    """
    check to see if the coordinates are within the viewbox
    adjusts if necessary
    """
        pass
