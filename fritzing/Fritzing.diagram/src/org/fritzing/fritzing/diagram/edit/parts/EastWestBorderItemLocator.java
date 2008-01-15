package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gmf.runtime.diagram.ui.editparts.DiagramEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.DiagramRootEditPart;
import org.eclipse.gmf.runtime.diagram.ui.figures.LayoutHelper;
import org.eclipse.gmf.runtime.draw2d.ui.mapmode.MapModeUtil;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditor;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorUtil;

public class EastWestBorderItemLocator extends PartBorderItemLocator {
	
	int positionConstants;
	Point offset;
	
	public EastWestBorderItemLocator(IFigure figure, PartEditPart parentEditPart, int positionConstants, Point offset) {
		super(figure, parentEditPart);
		this.offset = offset;
				
		this.positionConstants = positionConstants;
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
		
		if (offset != null) {
			x += offset.x;
			y += offset.y;
			return new Point(x, y);
		}
			
		int rpc = positionConstants;
		
		/*
		int offset = parentEditPart.getRotation() % 360;
		if (offset < 0) offset += 360;
				
		for (int i = 0; i < rotations.length; i++) {
			if (rpc == rotations[i]) {
				rpc = rotations[i + (offset / 90)];
				break;
			}
		}
		*/
		
		if (rpc == PositionConstants.WEST) {
			x += -borderItemSize.width - getBorderItemOffset().width;
			y += (parentFigureHeight - borderItemSize.height) / 2;
		} 
		else if (rpc == PositionConstants.EAST) {
			x += parentFigureWidth + getBorderItemOffset().width;
			y += (parentFigureHeight - borderItemSize.height) / 2;
		} 
		else if (rpc == PositionConstants.NORTH) {
			y -= borderItemSize.height;
			x += (parentFigureWidth - borderItemSize.width) / 2;						
		}
		else if (rpc == PositionConstants.SOUTH) {
			y += parentFigureHeight;
			x += (parentFigureWidth - borderItemSize.width) / 2;						
		}
		else if (rpc == PositionConstants.SOUTH_WEST) {
			y += parentFigureHeight;
		}
		else if (rpc == PositionConstants.SOUTH_EAST) {
			x += parentFigureWidth + getBorderItemOffset().width - borderItemSize.width;
			y += parentFigureHeight;
		}
		return new Point(x, y);
	}
	
	public int getCurrentSideOfParent() {
		return positionConstants;
	}
		
	static public int parseTerminalName(String name) {
		// can't rely on unique names--fix this		
		if (name == null) {	
			return PositionConstants.NONE;
		}
		if (name.equalsIgnoreCase("+")) {
			return PositionConstants.WEST;
		}
		if (name.equals("-")) {
			return PositionConstants.EAST;
		}
		if (name.equals("1")) {
			return PositionConstants.WEST;
		}
		else if (name.equals("2")) {
			return PositionConstants.EAST;
		}
		if (name.equals("T2")) {
			return PositionConstants.WEST;
		}
		else if (name.equals("T1")) {
			return PositionConstants.EAST;
		}
		else if (name.equals("W")) {
			return PositionConstants.SOUTH;
		}
		else if (name.equals("E")) {
			return PositionConstants.SOUTH_WEST;
		}
		else if (name.equals("B")) {
			return PositionConstants.SOUTH;
		}
		else if (name.equals("C")) {
			return PositionConstants.SOUTH_EAST;
		}
		
		// hack for the moment to distinguish terminals from Transistor and PowerTransistor parts
		else if (name.equals("e")) {
			return PositionConstants.SOUTH_EAST;
		}
		else if (name.equals("b")) {
			return PositionConstants.SOUTH_WEST;
		}
		else if (name.equals("c")) {
			return PositionConstants.SOUTH;
		}
		
		return PositionConstants.NONE;
	}


}
