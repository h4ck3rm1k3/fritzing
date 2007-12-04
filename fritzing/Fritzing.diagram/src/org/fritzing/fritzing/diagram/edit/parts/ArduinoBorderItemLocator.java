/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gmf.runtime.diagram.ui.figures.IBorderItemLocator;
import org.eclipse.gmf.runtime.diagram.ui.figures.LayoutHelper;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import java.util.EnumSet;

public class ArduinoBorderItemLocator implements IBorderItemLocator {
	
	static public enum TerminalPosition {
		AREF (15, PositionConstants.NORTH),
		GND (14, PositionConstants.NORTH),
		D0 (0, PositionConstants.NORTH),
		D1 (1, PositionConstants.NORTH),
		D2 (2, PositionConstants.NORTH),
		D3 (3, PositionConstants.NORTH),
		D4 (4, PositionConstants.NORTH),
		D5 (5, PositionConstants.NORTH),
		D6 (6, PositionConstants.NORTH),
		D7 (7, PositionConstants.NORTH),
		D8 (8, PositionConstants.NORTH),
		D9 (9, PositionConstants.NORTH),
		D10 (10, PositionConstants.NORTH),
		D11 (11, PositionConstants.NORTH),
		D12 (12, PositionConstants.NORTH),
		D13 (13, PositionConstants.NORTH),
		RESET (11, PositionConstants.SOUTH),
		_3V3 (10, PositionConstants.SOUTH),
		_5V (9, PositionConstants.SOUTH),
		Gnd (8, PositionConstants.SOUTH),
		Gnd1 (7, PositionConstants.SOUTH),
		VIN (6, PositionConstants.SOUTH),
		A0 (5, PositionConstants.SOUTH),
		A1 (4, PositionConstants.SOUTH),
		A2 (3, PositionConstants.SOUTH),
		A3 (2, PositionConstants.SOUTH),
		A4 (1, PositionConstants.SOUTH),
		A5 (0, PositionConstants.SOUTH),
		UNKNOWN (0, PositionConstants.NONE);
		
	    private final int index;   
	    private final int side;
	    
	    public int getIndex() {
	    	return index;
	    }
	    
	    public int getSide() {
	    	return side;
	    }

	    TerminalPosition(int index, int side)  {
			this.index = index;
			this.side = side;
		}
	    
	    private static TerminalPosition getX(TerminalPosition start, TerminalPosition stop, int index) {
	    	if (index < 0) return TerminalPosition.UNKNOWN;
	    	
	    	for (TerminalPosition tp : EnumSet.range(start, stop)) {
	    		if (index == 0) return tp;
	    		
	    		index--;
	    	}
	    	
	    	return TerminalPosition.UNKNOWN;
	    	
	    }
	    
	    public static TerminalPosition getD(int index) {
	    	return getX(TerminalPosition.D0, TerminalPosition.D13, index);
	    }
				
	    public static TerminalPosition getA(int index) {
	    	return getX(TerminalPosition.A0, TerminalPosition.A5, index);
	    }			
	}

	protected TerminalPosition terminalPosition;
	protected IFigure parentFigure;
	private Rectangle constraint = new Rectangle(0, 0, 0, 0);
	private Dimension borderItemOffset = new Dimension(2, 2);
		
	
	public ArduinoBorderItemLocator(IFigure parent, TerminalPosition terminalPosition) {
		parentFigure = parent;
		this.terminalPosition = terminalPosition;
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

	public int getCurrentSideOfParent() {
		return terminalPosition.getSide();
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
		
		// extra borderItemSize.width is for some space between terminals
		x += parentFigureWidth - ((terminalPosition.getIndex() + 1) * ((3 * borderItemSize.width) + getBorderItemOffset().width));
		if (terminalPosition.getSide() == PositionConstants.SOUTH) {
			y += parentFigureHeight;
		}
		else {
			y -= borderItemSize.height + getBorderItemOffset().height;
		}
		return new Point(x, y);
	}
	
	static public TerminalPosition parseTerminalName(String name) {
		if (name == null) {	
			return TerminalPosition.UNKNOWN;
		}
		if (name.equalsIgnoreCase("aref")) {
			return TerminalPosition.AREF;
		}
		if (name.equals("GND")) {
			return TerminalPosition.GND;
		}
		if (name.equals("Gnd")) {
			return TerminalPosition.Gnd;
		}
		else if (name.equals("Gnd1")) {
			return TerminalPosition.Gnd1;
		}
		else if (name.equalsIgnoreCase("reset")) {
			return TerminalPosition.RESET;
		}
		else if (name.equalsIgnoreCase("3v3")) {
			return TerminalPosition._3V3;
		}
		else if (name.equalsIgnoreCase("5v")) {
			return TerminalPosition._5V;
		}
		else if (name.equalsIgnoreCase("vin")) {
			return TerminalPosition.VIN;
		}
		else if (name.startsWith("D")) {
			try {
				int ix = Integer.parseInt(name.substring(1));
				return TerminalPosition.getD(ix);
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}					
		}
		else if (name.startsWith("A")) {
			try {
				int ix = Integer.parseInt(name.substring(1));
				return TerminalPosition.getA(ix);
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}					
		}
		
		
		return TerminalPosition.UNKNOWN;

	}
}
