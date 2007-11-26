/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;


import org.eclipse.gef.EditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editparts.AbstractBorderedShapeEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IRotatableEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.PopupBarEditPolicy;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.diagram.edit.policies.RotatableNonresizableShapeEditPolicy;

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
	public boolean isRotatable() {
		return true;
	}
	
	/**
	 * @generated NOT
	 */
	protected void createDefaultEditPolicies() {
		super.createDefaultEditPolicies();
		
		// removing these polices removes popups.  There are two popups, one from the diagramming assistant (CONNECTION_HANDLES_ROLE),
		// which popups up a pair of connection boxes; the other (POPUPBAR_ROLE) pops up child items that can be added to the diagram
		removeEditPolicy(org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles.POPUPBAR_ROLE);
		removeEditPolicy(org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles.CONNECTION_HANDLES_ROLE);
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


}
