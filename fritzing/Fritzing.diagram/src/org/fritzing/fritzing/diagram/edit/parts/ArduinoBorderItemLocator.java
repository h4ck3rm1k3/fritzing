/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gmf.runtime.diagram.ui.figures.LayoutHelper;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import java.util.EnumSet;
import java.util.Hashtable;

public class ArduinoBorderItemLocator extends PartBorderItemLocator {
	
	protected Point offset;
	
		
	public ArduinoBorderItemLocator(IFigure parent, PartEditPart parentPart, Point p) {
		super(parent, parentPart);
		this.offset = p;
		borderItemOffset.height = this.borderItemOffset.width = 2;	
	}
	

	protected Point getPreferredLocation(IFigure borderItem) {
		Rectangle bounds = getParentBorder();
		int parentFigureWidth = bounds.width;
		int parentFigureHeight = bounds.height;
		int parentFigureX = bounds.x;
		int parentFigureY = bounds.y;
		int x = parentFigureX;
		int y = parentFigureY;

//		Dimension borderItemSize = getSize(borderItem);
//		
//		// extra borderItemSize.width is for some space between terminals
//		x += parentFigureWidth - ((terminalPosition.getIndex() + 1) * ((3 * borderItemSize.width) + getBorderItemOffset().width));
//		if (terminalPosition.getSide() == PositionConstants.SOUTH) {
//			y += parentFigureHeight;
//		}
//		else {
//			y -= borderItemSize.height + getBorderItemOffset().height;
//		}
		
		if (offset != null) {
			x += offset.x;
			y += offset.y;
		}
		
		return new Point(x, y);
	}
	
}
