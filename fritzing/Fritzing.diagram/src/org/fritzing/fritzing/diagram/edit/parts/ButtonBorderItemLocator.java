package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gmf.runtime.diagram.ui.figures.LayoutHelper;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;


public class ButtonBorderItemLocator extends PartBorderItemLocator {
	
	int terminalPosition;
	
	public ButtonBorderItemLocator(IFigure figure, PartEditPart parentPart, int pos) {
		super(figure, parentPart);
		terminalPosition = pos;
	}

	protected Point getPreferredLocation(IFigure borderItem) {
		Rectangle bounds = getParentBorder();
		int parentFigureWidth = bounds.width;
		int parentFigureHeight = bounds.height;
		int parentFigureX = bounds.x;
		int parentFigureY = bounds.y;
		int x = parentFigureX;
		int y = parentFigureY;

		Dimension borderItemSize = getSize(borderItem);

		if (terminalPosition == PositionConstants.NORTH_WEST) {
			x += -borderItemSize.width - getBorderItemOffset().width;
		} 
		else if (terminalPosition == PositionConstants.NORTH_EAST) {
			x += parentFigureWidth + getBorderItemOffset().width;
		} 
		else if (terminalPosition == PositionConstants.SOUTH_WEST) {
			y += parentFigureHeight - borderItemSize.height - getBorderItemOffset().height;
			x += -borderItemSize.width - getBorderItemOffset().width;
		} 
		else if (terminalPosition == PositionConstants.SOUTH_EAST) {
			x += parentFigureWidth + getBorderItemOffset().width;
			y += parentFigureHeight - borderItemSize.height - getBorderItemOffset().height;
		}
		return new Point(x, y);
	}
	
	public int getCurrentSideOfParent() {
		return terminalPosition;
	}

}
