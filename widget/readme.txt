SVG Viewer for fritzing.
ver. 03.3, 27th July 2012.

dimitar.ruszev@gmail.com

Scripts from the net:

1. The following jquery svg plugin's source is:

http://keith-wood.name/svg.html

2. the svg pan and zoom script is hosted here: 

https://github.com/talos/jquery-svgpan

3. The bounding box script was taken from here:

http://my.opera.com/MacDev_ed/blog/2009/01/21/getting-boundingbox-of-svg-elements

4. These tutorials helped me understand the svg format better:

http://www.petercollingridge.co.uk/interactive-svg-components

(presumably has this guy other interesting projects as well)

General notes:

First of all, please excuse my eventual sloppy code; it's not intended carelessness but lack of javascript coding experience and insufficient knowledge of best practices.

The challenge of navigating an svg lies merely at coordinate transformations. Source (4) proposes a simple way to do panning / zooming / hover; however you can't really get back to jquery (i.e. you have to do all the drawing on the svg's "local level") and even then you will be having a hard time positioning new elements (in contrast to modifying existing ones, which is plain and simple.) Thats's why i had to use source (3) and (4).