/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;


import java.util.Iterator;

import org.eclipse.draw2d.PositionConstants;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editparts.AbstractBorderedShapeEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IRotatableEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.PopupBarEditPolicy;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.diagram.edit.policies.RotatableNonresizableShapeEditPolicy;
import org.eclipse.emf.common.notify.Notification;
import org.eclipse.emf.ecore.impl.EAttributeImpl;

/**
 * @generated NOT
 */
class PartEditPart extends AbstractBorderedShapeEditPart implements IRotatableEditPart 
{
	/**
	 * @generated NOT
	 */
	public PartEditPart(View view) {
		super(view);
	}
	
	
	/**
	 * @generated NOT
	 */
	public int getTerminalNamePosition(Terminal2EditPart terminal, int defaultPosition) {
		return defaultPosition;
	}

	/**
	 * @generated NOT
	 */
	public boolean isRotatable() {
		return true;
	}
	
	/**
	 * @generated NOT
	 */
	protected void createDefaultEditPolicies() {
		super.createDefaultEditPolicies();

		// place to add or remove policies
		// POPUP_BAR and CONNECTOR_HANDLES are disabled by default in preferences
	}
	
	public EditPolicy getPrimaryDragEditPolicy() {
		/*
		EditPolicy result = super.getPrimaryDragEditPolicy();
		if (result instanceof ResizableEditPolicy) {
			ResizableEditPolicy ep = (ResizableEditPolicy) result;
			ep.setResizeDirections(PositionConstants.NONE);
		}
		return result;
		*/
		
		return new RotatableNonresizableShapeEditPolicy();
	}
	
	
	protected NodeFigure createMainFigure() {
		return null;
	}
	
	protected int getEastWestBorderPositionConstants() {
		int count = 0;
		for (Iterator it = this.getChildren().iterator(); it.hasNext();  ) {
			if (it.next() instanceof Terminal2EditPart) count++;
		}
		
		switch (count) {
		case 1:
			return PositionConstants.EAST;
		case 2:
			return PositionConstants.WEST;
		default:
			return PositionConstants.NONE;
		
		}
	}

	protected void handleNotificationEvent(Notification notification) {
		
/*
		Object feature = notification.getFeature();
		System.out.println("got a notification " + 
				notification.getEventType() + " " + 
				notification.getNotifier().getClass().getName() 
				);
		        
		if (feature instanceof EAttributeImpl) {
			System.out.println("feature " + ((EAttributeImpl) feature).getName());			
		}
*/
		
		super.handleNotificationEvent(notification);
	}
}
