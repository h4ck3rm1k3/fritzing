/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;


import java.util.Iterator;

import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.gef.EditPart;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.Request;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.editpolicies.LayoutEditPolicy;
import org.eclipse.gef.editpolicies.NonResizableEditPolicy;
import org.eclipse.gef.requests.CreateRequest;
import org.eclipse.gmf.runtime.diagram.ui.editparts.AbstractBorderedShapeEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IBorderItemEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IRotatableEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.BorderItemSelectionEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.PopupBarEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.figures.BorderItemLocator;
import org.eclipse.gmf.runtime.diagram.ui.figures.IBorderItemLocator;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import org.eclipse.gmf.runtime.notation.View;
import org.eclipse.gmf.runtime.notation.impl.NodeImpl;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.diagram.edit.policies.RotatableNonresizableShapeEditPolicy;
import org.eclipse.emf.common.notify.Notification;
import org.eclipse.emf.ecore.EObject;
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
		
		Object feature = notification.getFeature();
		System.out.println("got a notification " + 
				notification.getEventType() + " " + 
				notification.getNotifier().getClass().getName() 
				);
		        
		if (feature instanceof EAttributeImpl) {
			System.out.println("feature " + ((EAttributeImpl) feature).getName());			
		}
		
		super.handleNotificationEvent(notification);
	}
	
	protected boolean addEastWestFixedChild(EditPart childEditPart) {
		if (childEditPart instanceof Terminal2EditPart) {
			int terminalPosition = PositionConstants.NONE;
			Object model = childEditPart.getModel();		
			if (model instanceof NodeImpl) {
				EObject eobject = ((NodeImpl) model).getElement();
				if (eobject instanceof Terminal) {
					String name = ((Terminal) eobject).getName();
					terminalPosition = EastWestBorderItemLocator.parseTerminalName(name);
				}
			}
			
			IBorderItemLocator locator = null;			
			if (terminalPosition != PositionConstants.NONE) {
				locator = new EastWestBorderItemLocator(getMainFigure(), this, terminalPosition);
			}
			else {
				locator =  new BorderItemLocator( getMainFigure(), terminalPosition);
			}
			getBorderedFigure().getBorderItemContainer().add(
				((Terminal2EditPart) childEditPart).getFigure(), locator);
			return true;
		}
				
		return false;
	}
	
	/**
	 * @generated NOT
	 */
	protected LayoutEditPolicy createLayoutEditPolicy() {
		LayoutEditPolicy lep = new LayoutEditPolicy() {

			protected EditPolicy createChildEditPolicy(EditPart child) {
				if ((child instanceof IBorderItemEditPart) && !(child instanceof Terminal2EditPart)) {
					return new BorderItemSelectionEditPolicy();
				}
				EditPolicy result = child
						.getEditPolicy(EditPolicy.PRIMARY_DRAG_ROLE);
				if (result == null) {
					result = new NonResizableEditPolicy();
				}
				return result;
			}

			protected Command getMoveChildrenCommand(Request request) {
				return null;
			}

			protected Command getCreateCommand(CreateRequest request) {
				return null;
			}
		};
		return lep;
	}

}
