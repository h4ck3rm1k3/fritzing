/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.IFigure;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.LayerConstants;
import org.eclipse.gef.Request;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.editparts.GridLayer;
import org.eclipse.gef.requests.CreateConnectionRequest;
import org.eclipse.gmf.runtime.diagram.ui.editparts.DiagramEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.ContainerNodeEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles;
import org.eclipse.gmf.runtime.diagram.ui.requests.RequestConstants;
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

		installEditPolicy(EditPolicy.GRAPHICAL_NODE_ROLE,
				new NoPopupContainerNodeEditPolicy());

		// POPUP_BAR and CONNECTOR_HANDLES are disabled by default in
		// preferences
	}

	/**
	 * @generated NOT
	 */
	public class NoPopupContainerNodeEditPolicy extends ContainerNodeEditPolicy {
		/**
		 * @generated NOT
		 */
		public Command getCommand(Request request) {
			if (RequestConstants.REQ_CONNECTION_END.equals(request.getType())
					&& request instanceof CreateConnectionRequest) {
				// don't popup a menu if the user drags out a wire from a terminal and drops it on the sketch
				return null;
			}

			return super.getCommand(request);
		}

	}
}
