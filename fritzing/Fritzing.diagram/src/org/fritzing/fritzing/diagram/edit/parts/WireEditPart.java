/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.Connection;
import org.eclipse.gef.EditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.ConnectionNodeEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles;
import org.eclipse.gmf.runtime.draw2d.ui.figures.PolylineConnectionEx;
import org.eclipse.gmf.runtime.draw2d.ui.figures.WrapLabel;
import org.eclipse.gmf.runtime.notation.View;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.widgets.Display;
import org.fritzing.fritzing.diagram.edit.policies.WireItemSemanticEditPolicy;

/**
 * @generated
 */
public class WireEditPart extends ConnectionNodeEditPart {

	/**
	 * @generated
	 */
	public static final int VISUAL_ID = 4001;

	/**
	 * @generated
	 */
	public WireEditPart(View view) {
		super(view);
	}

	/**
	 * @generated
	 */
	protected void createDefaultEditPolicies() {
		super.createDefaultEditPolicies();
		installEditPolicy(EditPolicyRoles.SEMANTIC_ROLE,
				new WireItemSemanticEditPolicy());
	}

	/**
	 * @generated
	 */
	protected boolean addFixedChild(EditPart childEditPart) {
		if (childEditPart instanceof WireNameEditPart) {
			((WireNameEditPart) childEditPart).setLabel(getPrimaryShape()
					.getFigureWireNameFigure());
			return true;
		}
		return false;
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
	 * Creates figure for this edit part.
	 * 
	 * Body of this method does not depend on settings in generation model
	 * so you may safely remove <i>generated</i> tag and modify it.
	 * 
	 * @generated
	 */

	protected Connection createConnectionFigure() {
		return new WireFigure();
	}

	/**
	 * @generated
	 */
	public WireFigure getPrimaryShape() {
		return (WireFigure) getFigure();
	}

	/**
	 * @generated
	 */
	public class WireFigure extends PolylineConnectionEx {

		/**
		 * @generated
		 */
		private WrapLabel fFigureWireNameFigure;

		/**
		 * @generated
		 */
		public WireFigure() {
			this.setLineWidth(3);

			createContents();
		}

		/**
		 * @generated
		 */
		private void createContents() {

			fFigureWireNameFigure = new WrapLabel();
			fFigureWireNameFigure.setText("..");

			fFigureWireNameFigure.setFont(FFIGUREWIRENAMEFIGURE_FONT);

			this.add(fFigureWireNameFigure);

		}

		/**
		 * @generated
		 */
		public WrapLabel getFigureWireNameFigure() {
			return fFigureWireNameFigure;
		}

	}

	/**
	 * @generated
	 */
	static final Font FFIGUREWIRENAMEFIGURE_FONT = new Font(Display
			.getCurrent(),
			Display.getDefault().getSystemFont().getFontData()[0].getName(), 8,
			SWT.NORMAL);

}
