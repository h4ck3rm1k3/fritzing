/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.IFigure;
import org.eclipse.gef.LayerConstants;
import org.eclipse.gef.editparts.GridLayer;
import org.eclipse.gmf.runtime.diagram.ui.editparts.DiagramEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles;
import org.eclipse.gmf.runtime.notation.View;
import org.eclipse.swt.graphics.Color;
import org.fritzing.fritzing.diagram.edit.policies.SketchCanonicalEditPolicy;
import org.fritzing.fritzing.diagram.edit.policies.SketchItemSemanticEditPolicy;

/**
 * @generated
 */
public class SketchEditPart extends DiagramEditPart {

	/**
	 * @generated
	 */
	public final static String MODEL_ID = "FritzingPhysical"; //$NON-NLS-1$

	/**
	 * @generated
	 */
	public static final int VISUAL_ID = 1000;

	/**
	 * @generated
	 */
	public SketchEditPart(View view) {
		super(view);
	}

	/**
	 * @generated NOT
	 */
	protected void createDefaultEditPolicies() {
		super.createDefaultEditPolicies();
		installEditPolicy(EditPolicyRoles.SEMANTIC_ROLE,
				new SketchItemSemanticEditPolicy());
		installEditPolicy(EditPolicyRoles.CANONICAL_ROLE,
				new SketchCanonicalEditPolicy());

		// POPUP_BAR and CONNECTOR_HANDLES are disabled by default in preferences
	}

	/**
	 * @generated NOT
	 */
	protected IFigure createFigure() {
	      IFigure fig = super.createFigure();
	      fig.setBackgroundColor(THIS_BACK);
	      fig.setOpaque(true);
	      
	      return fig;
	   }

	/**
	 * @generated NOT
	 */
	static final Color THIS_BACK = new Color(null, 204, 204, 204);
}
