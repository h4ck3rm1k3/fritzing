package org.fritzing.pcb.edit.parts;

import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.PointList;
import org.eclipse.draw2d.geometry.PrecisionRectangle;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gef.GraphicalEditPart;
import org.eclipse.gef.handles.HandleBounds;
import org.eclipse.gmf.runtime.diagram.ui.editparts.DiagramEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.DiagramRootEditPart;
import org.eclipse.gmf.runtime.diagram.ui.figures.LayoutHelper;
import org.eclipse.gmf.runtime.draw2d.ui.mapmode.MapModeUtil;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import org.fritzing.pcb.edit.policies.RotatableNonresizableShapeEditPolicy;
import org.fritzing.pcb.part.FritzingDiagramEditor;
import org.fritzing.pcb.part.FritzingDiagramEditorUtil;

public class EastWestBorderItemLocator extends PartBorderItemLocator {
	
	Point offset;
	Dimension originalSize;
	
	public EastWestBorderItemLocator(IFigure figure, PartEditPart parentEditPart, Point offset, Dimension originalSize) {
		super(figure, parentEditPart);
		this.offset = offset;
		this.originalSize = originalSize;
	}
	
	protected Point getPreferredLocation(IFigure borderItem) {
		Rectangle bounds = getParentBorder();
		int x = bounds.x;
		int y = bounds.y;
		
//		System.out.println("x:" + x + " y:" + y + 
//				" offsetx:" + offset.x + " offsety:" + offset.y +
//				" origw:" + originalSize.width + " origh:" + originalSize.height +
//				" boundsw:" + bounds.width + " boundsh:" + bounds.height);
		
		int degrees = parentEditPart.getRotation();
		if (degrees == 0) {		
			x += offset.x;
			y += offset.y;
		}
		else {
			double centerX = (originalSize.width / 2.0);
			double centerY = (originalSize.height / 2.0);			
			double ocx = offset.x - centerX;
			double ocy = offset.y - centerY;
 
			double rad = Math.toRadians(degrees);
			double dx = ocx * Math.cos(rad) + ocy * Math.sin(rad);
			double dy = -ocx * Math.sin(rad) + ocy * Math.cos(rad);
			
			
			double ndx = 0;
			double ndy = 0;
			switch (degrees) {
			case 45:
				ndx = 66;
				ndy = -132;
				break;
			case 90:
				ndx = 0;
				ndy = -264;
				break;
			case 135:
				ndx = -132;
				ndy = -264;
				break;
			case 180:
				ndx = -264;
				ndy = -264;
				break;
			case 225:
				ndx = -264;
				ndy = -132;
				break;
			case 270:
				ndx = -264;
				ndy = 0;
				break;
			case 315:
				ndx = -132;
				ndy = 66;
				break;
			}
			
//			System.out.println("ndx:" + ndx + " " + ndy + " " + degrees);			
//			System.out.println("xrot:" + (Math.cos(360 - rad) + Math.sin(360 - rad)) + " yrot:" + (-Math.sin(360 - rad) + Math.cos(360 - rad)));

			x = bounds.x + (int) ((bounds.width / 2.0) + dx + ndx);
			y = bounds.y + (int) ((bounds.height / 2.0) + dy + ndy);						
			
//			int centerX = -(originalSize.width / 2);
//			int centerY = -(originalSize.height / 2);			
//			
//			double rad = Math.toRadians(degrees);
//			double dx = offset.x * Math.cos(rad) + offset.y * Math.sin(rad);
//			double dy = -offset.x * Math.sin(rad) + offset.y * Math.cos(rad);
//						
//			double cx = centerX * Math.cos(rad) + centerY * Math.sin(rad);
//			double cy = -centerX * Math.sin(rad) + centerY * Math.cos(rad);
//			System.out.println("cenx:" + centerX + " ceny:" + centerY +  " dx:" + dx + " dy:" + dy + " cx:" + cx + " cy:" + cy);
//		
//			int insetX = (bounds.width - originalSize.width) / 2;
//			int insetY = (bounds.height - originalSize.height) / 2;
//			
//			//x += dx + cx - centerX + insetX;
//			//y += dy + cy - centerY + insetY;
//			
//			x += dx + cx + (bounds.width / 2);
//			y += dy + cy + (bounds.height / 2);
				
			
			
//			Rectangle bounds2 = ((HandleBounds) parentEditPart.getFigure()).getHandleBounds();
//			// now use the part's actual size and center it in the current bounding area
//		    Dimension originalSize = parentEditPart.getPartDefinition().getSize();
//		    bounds2.x += (bounds2.width - originalSize.width) / 2;
//		    bounds2.y += (bounds2.height - originalSize.height) / 2;
//		    bounds2.height = originalSize.height;
//		    bounds2.width = originalSize.width;		     
//			PrecisionRectangle pr = new PrecisionRectangle(bounds2);
//			
//			parentEditPart.getFigure().translateToAbsolute(pr);			        
//			double[][] rotated = RotatableNonresizableShapeEditPolicy.rotatePrecisionRect(degrees, pr);
//
//			double rad = Math.toRadians(degrees);
//			double dx = offset.x * Math.cos(rad) + offset.y * Math.sin(rad);
//			double dy = -offset.x * Math.sin(rad) + offset.y * Math.cos(rad);
//			
//			x = (int) (parentEditPart.DPtoLP((int) (rotated[0][0] - rotated[6][0])) + dx);
//			y = (int) (parentEditPart.DPtoLP((int) (rotated[0][1] - rotated[6][1])) + dy);
				
//			System.out.println("dx:" + dx + " dy:" + dy + " rx:" + rotated[0][0] + " ry:" + rotated[0][1]);

		}
		return new Point(x, y);
	}
	
	public int getCurrentSideOfParent() {
		return PositionConstants.NONE;
	}
		


}
