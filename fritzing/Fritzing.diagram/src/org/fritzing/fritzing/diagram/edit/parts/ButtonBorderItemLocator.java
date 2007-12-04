package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gmf.runtime.diagram.ui.figures.LayoutHelper;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import org.eclipse.gmf.runtime.diagram.ui.figures.IBorderItemLocator;

// non-deprecated version breaks the code
// import org.eclipse.gmf.runtime.draw2d.ui.figures.IBorderItemLocator;
 
public class ButtonBorderItemLocator implements IBorderItemLocator {
	
	int terminalPosition;
	IFigure parentFigure;
	private Rectangle constraint = new Rectangle(0, 0, 0, 0);
	private Dimension borderItemOffset = new Dimension(1, 1);
	
	public ButtonBorderItemLocator(IFigure figure, int positionConstants, int pos) {
		parentFigure = figure;
		terminalPosition = pos;
	}

	public Rectangle getValidLocation(Rectangle proposedLocation, IFigure borderItem) {
		Point p = getPreferredLocation(borderItem);
		Dimension d = getSize(borderItem);
		return new Rectangle(p.x, p.y, d.width, d.height);
	}
	
	protected IFigure getParentFigure() {
		return parentFigure;
	}
	
	protected Rectangle getParentBorder() {
		Rectangle bounds = getParentFigure().getBounds().getCopy();
		if (getParentFigure() instanceof NodeFigure) {
			bounds = ((NodeFigure) getParentFigure()).getHandleBounds()
				.getCopy();
		}
		return bounds;
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

	protected Point getPreferredLocation(int side, IFigure borderItem) {
		return getPreferredLocation(borderItem);
	}
	
	protected final Dimension getSize(IFigure borderItem) {
        Dimension size = getConstraint().getSize();
        if (LayoutHelper.UNDEFINED.getSize().equals(size)) {
        	size = borderItem.getPreferredSize();
        }
        return size;
	}

	public int getCurrentSideOfParent() {
		return terminalPosition;
	}

	public Dimension getBorderItemOffset() {
		return borderItemOffset;
	}
	
	protected Rectangle getConstraint() {
		return constraint;
	}
	
	public void setConstraint(Rectangle theConstraint) {
		this.constraint = theConstraint;

		getParentFigure().revalidate();
	}

	public void relocate(IFigure borderItem) {
        Dimension size = getSize(borderItem);

		Point ptNewLocation = getPreferredLocation(borderItem);
		borderItem.setLocation(ptNewLocation);
		borderItem.setSize(size);

	}

}
