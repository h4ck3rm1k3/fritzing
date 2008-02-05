/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.edit.policies;

import java.io.InputStream;
import java.util.HashMap;
import java.util.List;

import org.eclipse.core.runtime.FileLocator;
import org.eclipse.core.runtime.Path;
import org.eclipse.draw2d.Border;
import org.eclipse.draw2d.ColorConstants;
import org.eclipse.draw2d.Graphics;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.LineBorder;
import org.eclipse.draw2d.Polygon;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.PointList;
import org.eclipse.draw2d.geometry.PrecisionRectangle;
import org.eclipse.gef.GraphicalEditPart;
import org.eclipse.gef.Handle;
import org.eclipse.gef.Request;
import org.eclipse.gef.handles.AbstractHandle;
import org.eclipse.gef.requests.ChangeBoundsRequest;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.RotatableShapeEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.internal.handles.RotateHandle;
import org.eclipse.gmf.runtime.diagram.ui.internal.tools.RotateTracker;
import org.eclipse.gmf.runtime.draw2d.ui.figures.FigureUtilities;
import org.eclipse.gmf.runtime.draw2d.ui.mapmode.IMapMode;
import org.eclipse.gmf.runtime.draw2d.ui.mapmode.MapModeUtil;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Cursor;
import org.eclipse.swt.graphics.ImageData;
import org.eclipse.swt.graphics.RGB;
import org.fritzing.pcb.edit.commands.SetRotationCommand;
import org.fritzing.pcb.edit.parts.PartEditPart;
import org.fritzing.pcb.edit.requests.FritzingRotateShapeRequestEx;
import org.fritzing.pcb.edit.requests.RotateShapeRequestEx;
import org.fritzing.pcb.part.FritzingDiagramEditorPlugin;
import org.fritzing.pcb.utils.RotateDing;
import org.eclipse.draw2d.geometry.Rectangle;

/**
 * @generated NOT
 */
public class RotatableNonresizableShapeEditPolicy extends RotatableShapeEditPolicyEx {
	public static HashMap<Integer, Cursor> rotateCursors = new HashMap<Integer, Cursor>();
	static RGB handleRGB = new RGB(0x51, 0x8a, 0x5b);
	static Color fillColor = new Color(null, handleRGB);
	static String cursorImageFileSuffix = ".gif";
	static Color rotateGhostFillColor = new Color(null, 63, 63, 63);	
	
	ChangeBoundsRequest feedbackRequest = null;
	
	/**
	 * @generated NOT
	 */
	public void setResizeDirections(int newDirections) {
		// superclass sets these to all 4 directions in createSelectionHandles
		// so get rid of them here
		super.setResizeDirections(0);
	}

	protected RotateShapeRequestEx getRequest(ChangeBoundsRequest request) {
		FritzingRotateShapeRequestEx req = new FritzingRotateShapeRequestEx(REQ_RESIZE_CHILDREN);
		if (request instanceof FritzingRotateShapeRequestEx) {
			req.setNewBounds(((FritzingRotateShapeRequestEx) request).getNewBounds());
		}
		return req;
	}

	/**
	 * @generated NOT
	 */
	protected List createSelectionHandles() {
		List l = super.createSelectionHandles();
		for (Object obj : l) {
			if(obj instanceof AbstractHandle) {
				AbstractHandle handle = (AbstractHandle) obj;
				Border border = handle.getBorder();
				if(border instanceof LineBorder) {
					LineBorder lborder = (LineBorder) border;
					lborder.setWidth(2);
					lborder.setColor(fillColor);
				}
			}
		}
		
		return l;
	}
	
	/**
	 * @generated NOT
	 */
	protected Handle createRotationHandle(GraphicalEditPart owner, int direction) {
		String prefix = "";
		int hotX;
		int hotY;
		switch (direction) {
		case PositionConstants.SOUTH_EAST:
			hotY = 0;
			hotX = 0;
			prefix = "rotateSE";
			break;
		case PositionConstants.NORTH_EAST:
			hotY = 1;
			hotX = 0;
			prefix = "rotateNE";
			break;
		case PositionConstants.SOUTH_WEST:
			hotY = 0;
			hotX = 1;
			prefix = "rotateSW";
			break;
		case PositionConstants.NORTH_WEST:
			hotY = 1;
			hotX = 1;
			prefix = "rotateNW";
			break;
		default:
			direction = PositionConstants.SOUTH_EAST;
			hotY = 0;
			hotX = 0;
			prefix = "rotateSE";
		}
		
		Integer key = new Integer(direction);
		Cursor rotateCursor = rotateCursors.get(key);
		if (rotateCursor == null) {
			try {
				InputStream stream = FileLocator.openStream(
						FritzingDiagramEditorPlugin.getInstance().getBundle(), 
						new Path("icons/cursors/" + prefix + cursorImageFileSuffix), 
						false);
				if (stream != null) {		
					ImageData imageData = new ImageData(stream);
					if (hotY == 1) hotY = imageData.height - 1;
					if (hotX == 1) hotX = imageData.width - 1;
					rotateCursor = new Cursor(null, imageData, hotX, hotY);
					rotateCursors.put(key, rotateCursor);
				}
			}
			catch (Exception ex ) {
				FritzingDiagramEditorPlugin
				.getInstance()
				.logError( "Unable to create cursor: " + ex.getMessage(), ex);			
			}
			
		}
		
		
		RotateHandle handle = new RotateHandle(owner, direction) {
			protected Color getFillColor() {
				return fillColor;
			}			
		};
				
		if (rotateCursor != null) {
			handle.setCursor(rotateCursor);
		}
					
		handle.setDragTracker(new CustomCursorRotateTracker(owner, direction, rotateCursor));
		
				
		return handle;
				
	}
	
	protected IFigure createDragSourceFeedbackFigure() {
		if (!(feedbackRequest instanceof FritzingRotateShapeRequestEx)) {
			return super.createDragSourceFeedbackFigure();
		}
				
		// Use a ghost rectangle for feedback
		Polygon p = new Polygon();
		p.setFill(true);
		p.setOutline(true);
		
		setInitialPoints(p);

		//FigureUtilities.makeGhostShape(p);
		p.setBackgroundColor(rotateGhostFillColor);
		p.setFillXOR(true);
		p.setOutlineXOR(true);

		p.setLineStyle(Graphics.LINE_DOT);
		p.setForegroundColor(ColorConstants.white);
		//p.setBounds(r);
		addFeedback(p);
		return p;
	}
	
	/**
	 * @generated NOT
	 */	
	protected void setInitialPoints(Polygon p) {
		setPoints(p, 0, 0);
	}
	
	protected void showChangeBoundsFeedback(ChangeBoundsRequest request) {
		// If the figure is being rotated draw the rotation feedback
		feedbackRequest = request;
		if (request instanceof FritzingRotateShapeRequestEx) {
			// Get current feedback
			IFigure feedbackFigure = getDragSourceFeedbackFigure();
			int rotationDirection = getRotationDirection(request);
			((FritzingRotateShapeRequestEx) request).setRotationDirection(rotationDirection);
			if (doRotation(request)) {
				int fromAngle = SetRotationCommand.getAngle(request.getResizeDirection());
				int toAngle = SetRotationCommand.getAngle(rotationDirection);
				
				//System.out.println("from:" + request.getResizeDirection() + " to:" + rotationDirection);
																
				((FritzingRotateShapeRequestEx) request).setNewBounds(setPoints((Polygon) feedbackFigure, fromAngle, toAngle));
				

			}
			else {
				setInitialPoints((Polygon) feedbackFigure);
			}
		}
		else {
			// otherwise the figure is being resized
			super.showChangeBoundsFeedback(request);
		}
	}
	
	Rectangle setPoints(Polygon p, int fromAngle, int toAngle) {
        GraphicalEditPart part = (GraphicalEditPart) getHost();
        if (!(part instanceof PartEditPart)) return null;

 		PrecisionRectangle r = new PrecisionRectangle(getInitialFeedbackBounds());
		getHostFigure().translateToAbsolute(r);
		
		p.translateToRelative(r);

		int diff = toAngle - fromAngle;
		if (diff < 0) diff += 360;
		        
        int rot = ((PartEditPart) part).getRotation();  
		diff += rot;
					
		RotateDing rotateDing = RotateDing.rotatePrecisionRect(diff, r);
		rotateDing.adjustCenter();
				
		PointList pl = new PointList();
		pl.addPoint((int) rotateDing.p1x, (int) rotateDing.p1y);
		pl.addPoint((int) rotateDing.p2x, (int) rotateDing.p2y);
		pl.addPoint((int) rotateDing.p3x, (int) rotateDing.p3y);
		pl.addPoint((int) rotateDing.p4x, (int) rotateDing.p4y);
		pl.addPoint((int) rotateDing.p1x, (int) rotateDing.p1y);
		p.setPoints(pl);
		
		
        double zoom = ((PartEditPart) part).getPrimaryShape().getZoom();
 		
		Rectangle result = new Rectangle(p.getBounds());
		result.width = ((PartEditPart) part).DPtoLP((int) (result.width / zoom));
		result.height = ((PartEditPart) part).DPtoLP((int) (result.height / zoom));
		result.x = ((PartEditPart) part).DPtoLP((int) (result.x / zoom));
		result.y = ((PartEditPart) part).DPtoLP((int) (result.y / zoom));
		return result;
		
/*		
        IFigure feedback = getDragSourceFeedbackFigure();
        
        PrecisionRectangle rect = new PrecisionRectangle(getInitialFeedbackBounds().getCopy());
        getHostFigure().translateToAbsolute(rect);
        rect.translate(request.getMoveDelta());
        rect.resize(request.getSizeDelta());
        
        IFigure f = getHostFigure();
        Dimension min = f.getMinimumSize().getCopy();
        Dimension max = f.getMaximumSize().getCopy();
        IMapMode mmode = MapModeUtil.getMapMode(f);
        min.height = mmode.LPtoDP(min.height);
        min.width = mmode.LPtoDP(min.width);
        max.height = mmode.LPtoDP(max.height);
        max.width = mmode.LPtoDP(max.width);
        
        if (min.width>rect.width)
            rect.width = min.width;
        else if (max.width < rect.width)
            rect.width = max.width;
        
        if (min.height>rect.height)
            rect.height = min.height;
        else if (max.height < rect.height)
            rect.height = max.height;
        
        feedback.translateToRelative(rect);
        feedback.setBounds(rect);
		
*/		
		
		
	}
	
	
	protected Rectangle getInitialFeedbackBounds() {
		 Rectangle r = super.getInitialFeedbackBounds();	
		 // now use the part's actual size and center it in the current bounding area
	     Dimension dim = ((PartEditPart) getHost()).getPartDefinition().getSize();
	     r.x += (r.width - dim.width) / 2;
	     r.y += (r.height - dim.height) / 2;
	     r.height = dim.height;
	     r.width = dim.width;
	     return r;
	}
}

class CustomCursorRotateTracker extends RotateTrackerEx
{
	int direction;
	Cursor cursor;
		
	public CustomCursorRotateTracker(GraphicalEditPart owner, int direction, Cursor cursor) {
		super(owner, direction);
		this.direction = direction;
		this.cursor = cursor;
	}
	
	protected Cursor getDefaultCursor() {
		return cursor;
	}
		
	protected boolean  handleButtonDown(int button) {
		// 
		boolean result = super.handleButtonDown(button);
		if (button == 1) {
			// tell it we're already dragging
			
			// setFlag makes movedPastThreshold return true
			//setFlag(FLAG_PAST_THRESHOLD, true);
			setFlag(1, true);
			
			handleDrag();
			handleDragStarted();
			handleDragInProgress();
		}
		return result;
	}
	
	protected Request createSourceRequest() {
		// have to pass the info from a previous request to the next request
		FritzingRotateShapeRequestEx request = new FritzingRotateShapeRequestEx(REQ_RESIZE);
		request.setResizeDirection(getResizeDirection());
		return request;
	}
	
}


