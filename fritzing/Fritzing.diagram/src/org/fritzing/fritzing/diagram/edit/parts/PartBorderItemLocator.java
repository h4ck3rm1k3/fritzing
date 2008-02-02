package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gmf.runtime.diagram.ui.figures.IBorderItemLocator;
import org.eclipse.gmf.runtime.diagram.ui.figures.LayoutHelper;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import org.eclipse.draw2d.PositionConstants;

//non-deprecated version breaks the code
//import org.eclipse.gmf.runtime.draw2d.ui.figures.IBorderItemLocator;


public class PartBorderItemLocator implements IBorderItemLocator {
	
	public static int[] rotations = { PositionConstants.NORTH, PositionConstants.EAST, PositionConstants.SOUTH, PositionConstants.WEST,
										PositionConstants.NORTH, PositionConstants.EAST, PositionConstants.SOUTH };

	protected IFigure parentFigure;
	protected Rectangle constraint = new Rectangle(0, 0, 0, 0);
	protected Dimension borderItemOffset = new Dimension(1, 1);
	protected PartEditPart parentEditPart;

	public PartBorderItemLocator(IFigure parent, PartEditPart parentEditPart) {
		parentFigure = parent;
		this.parentEditPart = parentEditPart;
	}

	public Rectangle getValidLocation(Rectangle proposedLocation, IFigure borderItem) {
		Point p = getPreferredLocation(borderItem);
		Dimension d = getSize(borderItem);
		return new Rectangle(p.x, p.y, d.width, d.height);
	}

	protected final Dimension getSize(IFigure borderItem) {
        Dimension size = getConstraint().getSize();
        if (LayoutHelper.UNDEFINED.getSize().equals(size)) {
        	size = borderItem.getPreferredSize();
        }
        return size;
	}

	public void relocate(IFigure borderItem) {
        Dimension size = getSize(borderItem);

		Point ptNewLocation = getPreferredLocation(borderItem);
		borderItem.setLocation(ptNewLocation);
		borderItem.setSize(size);
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

	protected Point getPreferredLocation(IFigure borderItem) {
		Rectangle bounds = getParentBorder();
		return new Point(bounds.x, bounds.y);
	}
	
	public int getCurrentSideOfParent() {
		return PositionConstants.NONE;
	}


}
