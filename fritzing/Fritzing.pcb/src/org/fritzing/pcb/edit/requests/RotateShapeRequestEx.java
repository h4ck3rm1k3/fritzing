/******************************************************************************
 * Copyright (c) 2004 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    IBM Corporation - initial API and implementation 
 ****************************************************************************/

package org.fritzing.pcb.edit.requests;

import org.eclipse.draw2d.PositionConstants;
import org.eclipse.gef.requests.ChangeBoundsRequest;


// RotateShapeRequestEx class originally attached to https://bugs.eclipse.org/bugs/show_bug.cgi?id=167316

/**
 * Provides support for shape rotations
 * Essentially, same as ChangeBoundsRequest with an extra variable that allows rotation
 * 
 * @author oboyko
 */
public class RotateShapeRequestEx
	extends ChangeBoundsRequest {
	
	// Rotate permission: true if rotation permitted
	private boolean rotate;
	
	// resizeDirection represents the direction the handle points,
	// while rotationDirection represents the final direction
	private int rotationDirection;
	
	/**
	 * Builds an instance of the request
	 * 
	 * @param type
	 */
	public RotateShapeRequestEx(Object type) {
		super(type);
		rotate = true; 
	}
	
	/**
	 * Sets the rotation permission 
	 * 
	 * @param rotate the <code>boolean</code> <code>true</code> if rotation is permitted, 
	 * <code>false</code> otherwise.
	 */
	public void setRotate(boolean rotate) {
		this.rotate = rotate;
	}
	
	/**
	 * Returns the rotation permission
	 * 
	 * @return <code>boolean</code> <code>true</code> if rotation is permitted, 
	 * <code>false</code> otherwise.
	 */
	public boolean shouldRotate() {
		return rotate;
	}

	/**
	 * Returns the final rotation direction. See also resizeDirection.
	 * @return the final rotation direction
	 */
	public int getRotationDirection() {
		return rotationDirection;
	}

	/**
	 * Sets the final rotation direction. See also resizeDirection.
	 * @param rotationDirection the new rotation direction to one of the PositionConstants.
	 */
	public void setRotationDirection(int rotationDirection) {
		this.rotationDirection = rotationDirection;
	}
	
	//
	
	private static int[] directionsInRotationOrder = {
		PositionConstants.EAST, PositionConstants.SOUTH_EAST,
		PositionConstants.SOUTH, PositionConstants.SOUTH_WEST,
		PositionConstants.WEST, PositionConstants.NORTH_WEST,
		PositionConstants.NORTH, PositionConstants.NORTH_EAST,
	};
	
	/**
	 * Rotates direction rotation times.
	 * @param direction the direction to rotate. One of the PositionConstants.
	 * @param rotation the number of times to rotate, positive is clockwise.
	 * @return the direction rotated rotation times
	 */
	public static int rotate(int direction, int rotation) {
		int i = 0;
		int numDirections = directionsInRotationOrder.length;
		while (i < numDirections) {
			if (directionsInRotationOrder[i] == direction) {
				break;
			}
		}
		i += rotation;
		if (i < 0) {
			i += numDirections;
		} else if (i >= numDirections) {
			i -= numDirections;
		}
		return directionsInRotationOrder[i];
	}
	
	/**
	 * Gets the rotation as the number of half quadrants (45 degrees). 
	 * @param startDirection
	 * @param endDirection
	 * @return the relative rotation 
	 */
	public static int getRotation(int startDirection, int endDirection) {
		int start = -1, end = -1;
		for (int i = 0; i < directionsInRotationOrder.length; i++) {
			if (directionsInRotationOrder[i] == startDirection) {
				start = i;
			}
			if (directionsInRotationOrder[i] == endDirection) {
				end = i;
			}
		}
		return end - start;
	}

	public int getRotation() {
		return getRotation(getResizeDirection(), getRotationDirection());
	}
}
