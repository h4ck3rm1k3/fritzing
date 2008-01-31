/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.IFigure;
import org.eclipse.emf.common.notify.Notification;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gef.EditPart;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.editpolicies.LayoutEditPolicy;
import org.eclipse.gmf.runtime.diagram.core.util.ViewUtil;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.CreationEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.DragDropEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import org.eclipse.gmf.runtime.notation.View;
import org.eclipse.swt.graphics.Color;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.LED;
import org.fritzing.fritzing.diagram.edit.PartDefinition;
import org.fritzing.fritzing.diagram.edit.policies.LEDCanonicalEditPolicy;
import org.fritzing.fritzing.diagram.edit.policies.LEDItemSemanticEditPolicy;
import org.fritzing.fritzing.diagram.part.FritzingVisualIDRegistry;

/**
 * @generated NOT
 */
public class LEDEditPart extends PartEditPart {

	/**
	 * @generated
	 */
	public static final int VISUAL_ID = 2001;

	/**
	 * @generated NOT
	 */
	public LEDEditPart(View view) {
		super(view);
	}

	/**
	 * @generated
	 */
	protected void createDefaultEditPolicies() {
		installEditPolicy(EditPolicyRoles.CREATION_ROLE,
				new CreationEditPolicy());

		super.createDefaultEditPolicies();
		installEditPolicy(EditPolicyRoles.SEMANTIC_ROLE,
				new LEDItemSemanticEditPolicy());
		installEditPolicy(EditPolicyRoles.DRAG_DROP_ROLE,
				new DragDropEditPolicy());
		installEditPolicy(EditPolicyRoles.CANONICAL_ROLE,
				new LEDCanonicalEditPolicy());
		installEditPolicy(EditPolicy.LAYOUT_ROLE, createLayoutEditPolicy());
		// XXX need an SCR to runtime to have another abstract superclass that would let children add reasonable editpolicies
		// removeEditPolicy(org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles.CONNECTION_HANDLES_ROLE);
	}

	/**
	 * @generated NOT
	 */
	protected LayoutEditPolicy createLayoutEditPolicy() {
		return super.createLayoutEditPolicy();
	}

	/**
	 * @generated NOT
	 */
	protected void handleNotificationEvent(Notification evt) {
		if (FritzingPackage.eINSTANCE.getLED_Color().equals(evt.getFeature())) {
			EObject led = ViewUtil.resolveSemanticElement((View) getModel());
			if (led instanceof LED) {
				int color = ((LED) led).getColor();
				getPrimaryShape().setColor(color);
			}
		} else {
			super.handleNotificationEvent(evt);
		}
	}

	/**
	 * @generated NOT
	 */
	protected IFigure createNodeShape() {
		LEDFigure figure = new LEDFigure(partDefinition);
		return primaryShape = figure;
	}

	/**
	 * @generated
	 */
	protected void addChildVisual(EditPart childEditPart, int index) {
		if (addFixedChild(childEditPart)) {
			return;
		}
		super.addChildVisual(childEditPart, -1);
	}

	/**
	 * @generated NOT
	 */
	protected boolean addFixedChild(EditPart childEditPart) {
		return addEastWestFixedChild(childEditPart);
	}

	/**
	 * @generated
	 */
	public LEDFigure getPrimaryShape() {
		return (LEDFigure) primaryShape;
	}

	/**
	 * @generated NOT
	 */
	protected NodeFigure createNodePlate() {
		return super.createNodePlate();
	}

	/**
	 * @generated NOT
	 */
	public EditPolicy getPrimaryDragEditPolicy() {

		return super.getPrimaryDragEditPolicy();
	}


	/**
	 * @generated
	 */
	public EditPart getPrimaryChildEditPart() {
		return getChildBySemanticHint(FritzingVisualIDRegistry
				.getType(LEDNameEditPart.VISUAL_ID));
	}

	/**
	 * @generated NOT
	 */
	public class LEDFigure extends PartFigure {

		/**
		 * @generated NOT
		 */
		public LEDFigure(PartDefinition partDefinition) {
			super(partDefinition);
		}

		/**
		 * @generated
		 */
		private boolean myUseLocalCoordinates = false;

		/**
		 * @generated
		 */
		protected boolean useLocalCoordinates() {
			return myUseLocalCoordinates;
		}

		/**
		 * @generated
		 */
		protected void setUseLocalCoordinates(boolean useLocalCoordinates) {
			myUseLocalCoordinates = useLocalCoordinates;
		}

		/**
		 * @generated NOT
		 */
		public void setColor(int color) {
			Color c = new Color(null, (color >> 16) & 255, (color >> 8) & 255,
					color & 255);
			this.setBackgroundColor(c);
			repaint();
		}

	}

}
