package org.fritzing.pcb.edit.requests;

import org.eclipse.draw2d.geometry.Rectangle;

public class FritzingRotateShapeRequestEx extends RotateShapeRequestEx {
	
	Rectangle newBounds;
	
	public FritzingRotateShapeRequestEx(Object type) {
		super(type);
	}
	
	public Rectangle getNewBounds() {
		return newBounds;
	}
	
	public void setNewBounds(Rectangle newBounds) {
		this.newBounds = newBounds;
		
//		if (newBounds != null) {
//			System.out.println("set new bounds " + newBounds.x + " " + newBounds.y + " " + newBounds.width + " " + newBounds.height);
//		}
	}
}
