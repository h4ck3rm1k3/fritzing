/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.Ellipse;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.RectangleFigure;
import org.eclipse.draw2d.StackLayout;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gef.EditPart;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.Request;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.editpolicies.LayoutEditPolicy;
import org.eclipse.gef.editpolicies.NonResizableEditPolicy;
import org.eclipse.gef.requests.CreateRequest;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IBorderItemEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IGraphicalEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.BorderItemSelectionEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.CreationEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.DragDropEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles;
import org.eclipse.gmf.runtime.diagram.ui.figures.BorderItemLocator;
import org.eclipse.gmf.runtime.draw2d.ui.figures.ConstrainedToolbarLayout;
import org.eclipse.gmf.runtime.draw2d.ui.figures.WrapLabel;
import org.eclipse.gmf.runtime.gef.ui.figures.DefaultSizeNodeFigure;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import org.eclipse.gmf.runtime.notation.View;
import org.eclipse.gmf.runtime.notation.impl.NodeImpl;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.widgets.Display;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.diagram.edit.policies.ButtonCanonicalEditPolicy;
import org.fritzing.fritzing.diagram.edit.policies.ButtonItemSemanticEditPolicy;
import org.fritzing.fritzing.diagram.part.FritzingVisualIDRegistry;

/**
 * @generated NOT
 */
public class ButtonEditPart extends PartEditPart {

	/**
	 * @generated NOT
	 */
	public static final String UPPER_LEFT_TERMINAL = "0p";

	/**
	 * @generated NOT
	 */
	public static final String UPPER_RIGHT_TERMINAL = "1p";

	/**
	 * @generated NOT
	 */
	public static final String LOWER_LEFT_TERMINAL = "0";

	/**
	 * @generated NOT
	 */
	public static final String LOWER_RIGHT_TERMINAL = "1";

	/**
	 * @generated
	 */
	public static final int VISUAL_ID = 2004;

	/**
	 * @generated
	 */
	protected IFigure contentPane;

	/**
	 * @generated
	 */
	protected IFigure primaryShape;

	/**
	 * @generated
	 */
	public ButtonEditPart(View view) {
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
				new ButtonItemSemanticEditPolicy());
		installEditPolicy(EditPolicyRoles.DRAG_DROP_ROLE,
				new DragDropEditPolicy());
		installEditPolicy(EditPolicyRoles.CANONICAL_ROLE,
				new ButtonCanonicalEditPolicy());
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
	 * @generated
	 */
	protected IFigure createNodeShape() {
		ButtonFigure figure = new ButtonFigure();
		return primaryShape = figure;
	}

	/**
	 * @generated
	 */
	public ButtonFigure getPrimaryShape() {
		return (ButtonFigure) primaryShape;
	}

	/**
	 * @generated NOT
	 */
	protected boolean addFixedChild(EditPart childEditPart) {
		if (childEditPart instanceof ButtonNameEditPart) {
			((ButtonNameEditPart) childEditPart).setLabel(getPrimaryShape()
					.getFigureButtonNameFigure());
			return true;
		}
		if (childEditPart instanceof Terminal2EditPart) {
			String name = null;
			Object model = childEditPart.getModel();
			int pos = 0;
			if (model instanceof NodeImpl) {
				EObject eobject = ((NodeImpl) model).getElement();
				if (eobject instanceof Terminal) {
					name = ((Terminal) eobject).getName();
					if (name == null) {
					} else if (name.equalsIgnoreCase(UPPER_LEFT_TERMINAL)) {
						pos = PositionConstants.NORTH_WEST;
					} else if (name.equalsIgnoreCase(UPPER_RIGHT_TERMINAL)) {
						pos = PositionConstants.NORTH_EAST;
					} else if (name.equalsIgnoreCase(LOWER_LEFT_TERMINAL)) {
						pos = PositionConstants.SOUTH_WEST;
					} else if (name.equalsIgnoreCase(LOWER_RIGHT_TERMINAL)) {
						pos = PositionConstants.SOUTH_EAST;
					}
				}
			}

			ButtonBorderItemLocator locator = new ButtonBorderItemLocator(
					getMainFigure(), this, pos);
			//BorderItemLocator locator = new BorderItemLocator(getMainFigure(), 0);
			getBorderedFigure().getBorderItemContainer().add(
					((Terminal2EditPart) childEditPart).getFigure(), locator);
			return true;
		}
		return false;
	}

	/**
	 * @generated NOT
	 */
	public int getTerminalNamePosition(Terminal2EditPart terminal2,
			TerminalName2EditPart namePart, int defaultPosition) {
		String name = null;
		Object model = terminal2.getModel();
		if (model instanceof NodeImpl) {
			EObject eobject = ((NodeImpl) model).getElement();
			if (eobject instanceof Terminal) {
				name = ((Terminal) eobject).getName();
			}
		}

		if (name == null)
			return defaultPosition;

		if (name.equalsIgnoreCase(UPPER_LEFT_TERMINAL)) {
			return PositionConstants.NORTH;
		} else if (name.equalsIgnoreCase(UPPER_RIGHT_TERMINAL)) {
			return PositionConstants.NORTH;
		} else if (name.equalsIgnoreCase(LOWER_LEFT_TERMINAL)) {
			return PositionConstants.SOUTH;
		} else if (name.equalsIgnoreCase(LOWER_RIGHT_TERMINAL)) {
			return PositionConstants.SOUTH;
		}

		return defaultPosition;

	}

	/**
	 * @generated
	 */
	protected boolean removeFixedChild(EditPart childEditPart) {

		if (childEditPart instanceof Terminal2EditPart) {
			getBorderedFigure().getBorderItemContainer().remove(
					((Terminal2EditPart) childEditPart).getFigure());
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
	 * @generated
	 */
	protected void removeChildVisual(EditPart childEditPart) {
		if (removeFixedChild(childEditPart)) {
			return;
		}
		super.removeChildVisual(childEditPart);
	}

	/**
	 * @generated
	 */
	protected IFigure getContentPaneFor(IGraphicalEditPart editPart) {

		if (editPart instanceof Terminal2EditPart) {
			return getBorderedFigure().getBorderItemContainer();
		}
		return super.getContentPaneFor(editPart);
	}

	/**
	 * @generated
	 */
	protected NodeFigure createNodePlate() {
		DefaultSizeNodeFigure result = new DefaultSizeNodeFigure(getMapMode()
				.DPtoLP(20), getMapMode().DPtoLP(20));
		return result;
	}

	/**
	 * @generated NOT
	 */
	public EditPolicy getPrimaryDragEditPolicy() {
		return super.getPrimaryDragEditPolicy();
	}

	/**
	 * Creates figure for this edit part.
	 * 
	 * Body of this method does not depend on settings in generation model
	 * so you may safely remove <i>generated</i> tag and modify it.
	 * 
	 * @generated
	 */
	protected NodeFigure createMainFigure() {
		NodeFigure figure = createNodePlate();
		figure.setLayoutManager(new StackLayout());
		IFigure shape = createNodeShape();
		figure.add(shape);
		contentPane = setupContentPane(shape);
		return figure;
	}

	/**
	 * Default implementation treats passed figure as content pane.
	 * Respects layout one may have set for generated figure.
	 * @param nodeShape instance of generated figure class
	 * @generated
	 */
	protected IFigure setupContentPane(IFigure nodeShape) {
		if (nodeShape.getLayoutManager() == null) {
			ConstrainedToolbarLayout layout = new ConstrainedToolbarLayout();
			layout.setSpacing(getMapMode().DPtoLP(5));
			nodeShape.setLayoutManager(layout);
		}
		return nodeShape; // use nodeShape itself as contentPane
	}

	/**
	 * @generated
	 */
	public IFigure getContentPane() {
		if (contentPane != null) {
			return contentPane;
		}
		return super.getContentPane();
	}

	/**
	 * @generated
	 */
	public EditPart getPrimaryChildEditPart() {
		return getChildBySemanticHint(FritzingVisualIDRegistry
				.getType(ButtonNameEditPart.VISUAL_ID));
	}

	/**
	 * @generated
	 */
	public class ButtonFigure extends RectangleFigure {

		/**
		 * @generated
		 */
		private WrapLabel fFigureButtonNameFigure;

		/**
		 * @generated
		 */
		public ButtonFigure() {

			this.setLayoutManager(new StackLayout());
			this.setForegroundColor(THIS_FORE);
			this.setBackgroundColor(THIS_BACK);
			this.setPreferredSize(new Dimension(getMapMode().DPtoLP(20),
					getMapMode().DPtoLP(20)));
			createContents();
		}

		/**
		 * @generated
		 */
		private void createContents() {

			Ellipse elli0 = new Ellipse();
			elli0.setBackgroundColor(ELLI0_BACK);

			this.add(elli0);

			fFigureButtonNameFigure = new WrapLabel();
			fFigureButtonNameFigure.setText("..");

			fFigureButtonNameFigure.setFont(FFIGUREBUTTONNAMEFIGURE_FONT);

			this.add(fFigureButtonNameFigure);

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
		 * @generated
		 */
		public WrapLabel getFigureButtonNameFigure() {
			return fFigureButtonNameFigure;
		}

	}

	/**
	 * @generated
	 */
	static final Color THIS_FORE = new Color(null, 0, 0, 0);

	/**
	 * @generated
	 */
	static final Color THIS_BACK = new Color(null, 128, 128, 128);

	/**
	 * @generated
	 */
	static final Color ELLI0_BACK = new Color(null, 192, 192, 192);

	/**
	 * @generated
	 */
	static final Font FFIGUREBUTTONNAMEFIGURE_FONT = new Font(Display
			.getCurrent(),
			Display.getDefault().getSystemFont().getFontData()[0].getName(), 8,
			SWT.NORMAL);

}
