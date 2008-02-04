/******************************************************************************
 * Copyright (c) 2004, 2006 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    IBM Corporation - initial API and implementation 
 ****************************************************************************/

package org.fritzing.fritzing.diagram.edit.policies;

import java.util.List;

import org.eclipse.draw2d.ColorConstants;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.PrecisionRectangle;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gef.GraphicalEditPart;
import org.eclipse.gef.Handle;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.requests.ChangeBoundsRequest;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.ResizableShapeEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.internal.handles.RotateHandle;
import org.eclipse.swt.graphics.Color;
import org.fritzing.fritzing.diagram.edit.requests.RotateShapeRequestEx;


//RotatableShapeEditPolicyEx class originally attached to https://bugs.eclipse.org/bugs/show_bug.cgi?id=167316


/**
 * A rotatable editpolicy for rotating fork and join itparts
 * It rotates the figure if diagonal handlers are dragged and
 * resizes the figure otherwise as defined by the superclass
 *  
 * @author oboyko
 */
public class RotatableShapeEditPolicyEx extends ResizableShapeEditPolicy {
	
	// how much should be the mice moved to rotate the figure
	private final static int DEFAULT_TOLERANCE = 6;

	/*
	 * Create the selection handles for edit parts that have Rotatable Edit Policy
	 * installed on them, i.e. Forks and Joins from Activity and State Machine diagrams
	 *  (non-Javadoc)
	 * @see org.eclipse.gef.editpolicies.SelectionHandlesEditPolicy#createSelectionHandles()
	 */
	protected List createSelectionHandles() {
        setResizeDirections(PositionConstants.EAST | PositionConstants.SOUTH | 
            PositionConstants.WEST | PositionConstants.NORTH);
        
        List selectionhandles = super.createSelectionHandles();
        GraphicalEditPart part = (GraphicalEditPart) getHost();
		
		selectionhandles.add(createRotationHandle(part, PositionConstants.SOUTH_EAST));
		selectionhandles.add(createRotationHandle(part, PositionConstants.SOUTH_WEST));
		selectionhandles.add(createRotationHandle(part, PositionConstants.NORTH_WEST));
		selectionhandles.add(createRotationHandle(part, PositionConstants.NORTH_EAST));
		return selectionhandles;
	}
	
	/**
	 * Create rotate handle with a rotate tracker
	 * @param owner the owner edit part
	 * @param direction the handle direction
	 * @return the handle
	 */
	protected Handle createRotationHandle(GraphicalEditPart owner, int direction) {
		RotateHandle handle = new RotateHandle(owner, direction) {
			protected Color getFillColor() {
				return ColorConstants.gray;
			}
		};
		handle.setDragTracker(
			new RotateTrackerEx(owner, direction));
		return handle;
	}
	
	/**
	 * Shows or updates feedback for a change bounds request that is seen as rotation
	 * @param request the request
	*/
	
//	private String ds(int direction, int mask, String name) {
//		return (direction & mask) > 0 ? name : "";
//	}
//	private String ds(int dir) {
//		return ds(dir, PositionConstants.NORTH, "north")
//		+ ds(dir, PositionConstants.SOUTH, "south")
//		+ ds(dir, PositionConstants.WEST, "west")
//		+ ds(dir, PositionConstants.EAST, "east");
//	}
	
	protected void showChangeBoundsFeedback(ChangeBoundsRequest request) {
		// If the figure is being rotated draw the rotation feedback
		if (isRotationRequired(request)) {
			// Get current feedback
			IFigure feedback = getDragSourceFeedbackFigure();
			if (request instanceof RotateShapeRequestEx) {
				((RotateShapeRequestEx)request).setRotationDirection(getRotationDirection(request));
			}
			if (doRotation(request)) {
				// Get the absolute coordinates for rotated figure
				PrecisionRectangle rect = getAbsoluteRotatedBounds(request);
				// Draw the rotated figure in the feedback
				feedback.translateToRelative(rect);
				feedback.setBounds(rect);
			}
			else {
				// Get the absolute coordinates for initial and rotated figure
				PrecisionRectangle initFigure = getAbsoluteInitialBounds();
				// Draw the initial figure in the feedback
				feedback.translateToRelative(initFigure);
				feedback.setBounds(initFigure);
			}
		}
		else {
			// otherwise the figure is being resized
			super.showChangeBoundsFeedback(request);
		}
	}
	
	/*
	 * Returns PrecisionRectangle obtained from the rotation by 90 deg. of an argument rectangle
	 * with respect to it's geometrical centre
	 * @param Rectangle r
	 * @return PrecisionRectangle rect obtained from rotation of r
	 */
	private PrecisionRectangle rotateRectangle(Rectangle r) {
		PrecisionRectangle rect = new PrecisionRectangle(r);
		if (isVertical(r)) {
			rect.setX(rect.preciseX-rect.preciseHeight/2.0+rect.preciseWidth/2.0);
			rect.setY(rect.preciseY+rect.preciseHeight/2.0-rect.preciseWidth/2.0);
		}
		else {
			rect.setX(rect.preciseX+rect.preciseWidth/2.0-rect.preciseHeight/2.0);
			rect.setY(rect.preciseY-rect.preciseWidth/2.0+rect.preciseHeight/2.0);
		}
		transposePrecisionRectangleSize(rect);
		return rect;
	}
	
	/*
	 * Check whether the bar (or figure) is vertical or horizontal
	 * @param Rectangle - the bounds of the figure
	 * @return true if figure is vertical, fasle if figure is horizontal
	 */
	private boolean isVertical(Rectangle r) {
		return r.height > r.width;
	}
	
	/*
	 * Transposes PrecisionRectangle's size
	 * @param PrecisionRectangle
	 * @return PrecisionRectangle with transposed size  
	 */
	private void transposePrecisionRectangleSize(PrecisionRectangle r) {
		double height = r.preciseHeight;
		r.setHeight(r.preciseWidth);
		r.setWidth(height);
	}
	
	/*
	 * Returns if figure must be rotated based on the info in the request, i.e. 
	 * diagonal resize direction and rotatable edit parts are selected.
	 * @param change bounds request
	 * @return true if figure must be rotated
	 */
	private boolean isRotationRequired(ChangeBoundsRequest request) {
		return request instanceof RotateShapeRequestEx ?  ((RotateShapeRequestEx) request).shouldRotate() : false; 
	}
	
	protected int getRotationDirection(ChangeBoundsRequest request) {
		PrecisionRectangle rect = getAbsoluteInitialBounds();
		double midX = rect.preciseX + rect.preciseWidth/2.0, midY = rect.preciseY + rect.preciseHeight/2.0;
		Point point = request.getLocation();
		double dx = point.x - midX, dy = point.y - midY;
		double adx = Math.abs(dx), ady = Math.abs(dy);
		if (adx < ady && adx * 2.0 < ady) {
			dx = 0.0;
		} else if (ady != 0 && ady < adx && ady * 2.0 < adx) {
			dy = 0.0;
		}
		int resizeDirection =
			(dx == 0.0 ? 0 : (dx < 0 ? PositionConstants.WEST : PositionConstants.EAST)) |
			(dy == 0.0 ? 0 : (dy < 0 ? PositionConstants.NORTH : PositionConstants.SOUTH));
		return resizeDirection;
	}

	/*
	 * Returns the command contribution for the given resize request. By default, the request
	 * is redispatched to the host's parent as a {@link
	 * org.eclipse.gef.RequestConstants#REQ_RESIZE_CHILDREN}.  The parent's editpolicies
	 * determine how to perform the resize based on the layout manager in use.
	 * @param request the resize request
	 * @return the command contribution obtained from the parent
	 * @see org.eclipse.gef.editpolicies.ResizableEditPolicy#getResizeCommand(org.eclipse.gef.requests.ChangeBoundsRequest)
	*/
	
	
	// jrc: added getRequest so we can subclass it more easily	
	protected RotateShapeRequestEx getRequest(ChangeBoundsRequest request) {
		return new RotateShapeRequestEx(REQ_RESIZE_CHILDREN);
	}
	
	protected Command getResizeCommand(ChangeBoundsRequest request) {
		// if the figure needs to be rotated set the command with the proper data
		if (isRotationRequired(request)) {
			// why create a new req here instead of reusing request
			RotateShapeRequestEx req = getRequest(request);
			req.setEditParts(getHost());
			
			// fake resizing and movement to resize the figure if mice is moved far enough
			if (doRotation(request)) {
				// Get the absolute coordinates for initial and rotated figure
				PrecisionRectangle rect = getAbsoluteRotatedBounds(request);
				PrecisionRectangle initFigure = getAbsoluteInitialBounds();
				req.setMoveDelta(new Point(rect.preciseX - initFigure.preciseX, rect.preciseY - initFigure.preciseY));
				req.setSizeDelta(new Dimension(rect.width - initFigure.width, rect.height - initFigure.height));
			}
			else {
				// otherwise SizeDelta and MoveDelta must be 0s
				req.setSizeDelta(new Dimension());
				req.setMoveDelta(new Point());
			}
			req.setLocation(request.getLocation());
			req.setExtendedData(request.getExtendedData());
			req.setResizeDirection(request.getResizeDirection());
			req.setRotationDirection(getRotationDirection(req));
			return getHost().getParent().getCommand(req);
		}
		else {
			// otherwise the figure is being resized
			return super.getResizeCommand(request);
		}
	}
	
	/*
	 * Based on the size delta from the request determines whether the EditPart must be rotated or
	 * remain as it is 
	 * 
	 *  jrc: made doRotation protected instead of private
	 */
	protected boolean doRotation(ChangeBoundsRequest request) {
		return (Math.abs(request.getSizeDelta().width) > DEFAULT_TOLERANCE || Math.abs(request.getSizeDelta().height) > DEFAULT_TOLERANCE) &&
			request.getResizeDirection() != getRotationDirection(request);
	}
	
	/*
	 * Returns the bounds of the initial figure in the absolute coordinates
	 */
	private PrecisionRectangle getAbsoluteInitialBounds() {
		// store the initial figure
		PrecisionRectangle initFigure = new PrecisionRectangle(getInitialFeedbackBounds().getCopy());
		getHostFigure().translateToAbsolute(initFigure);
		return initFigure;
	}
	
	/*
	 * Returns the bounds of the rotated initial figure with respect to its geometrical centre
	 * in absolute coordinates
	 */
	private PrecisionRectangle getAbsoluteRotatedBounds(ChangeBoundsRequest request) {
		// store the rotated figure
		Rectangle rect = getInitialFeedbackBounds();
		// get the difference in direction, to see if we need to rotate
		int directionDiff = request.getResizeDirection() ^ getRotationDirection(request);
		// if the direction has changed along both axes i.e. is 180 degrees,
		// we don't actually need to rotate, as the rectangle will be the same
		if (((directionDiff & PositionConstants.EAST_WEST) > 0) != ((directionDiff & PositionConstants.NORTH_SOUTH) > 0)) {
			rect = rotateRectangle(rect.getCopy().getCopy());
		}
		PrecisionRectangle preciseRect = new PrecisionRectangle(rect);
		getHostFigure().translateToAbsolute(preciseRect);
		return preciseRect;
	}
}