/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.Ellipse;
import org.eclipse.draw2d.FlowLayout;
import org.eclipse.draw2d.FreeformLayout;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.StackLayout;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gef.EditPart;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.Request;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.editpolicies.LayoutEditPolicy;
import org.eclipse.gef.editpolicies.NonResizableEditPolicy;
import org.eclipse.gef.editpolicies.ResizableEditPolicy;
import org.eclipse.gef.requests.CreateRequest;
import org.eclipse.gmf.runtime.diagram.ui.editparts.AbstractBorderedShapeEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IBorderItemEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IGraphicalEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.BorderItemSelectionEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.CreationEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.DragDropEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.ResizableShapeEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.XYLayoutEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.figures.BorderItemLocator;
import org.eclipse.gmf.runtime.draw2d.ui.figures.ConstrainedToolbarLayout;
import org.eclipse.gmf.runtime.draw2d.ui.figures.WrapLabel;
import org.eclipse.gmf.runtime.gef.ui.figures.DefaultSizeNodeFigure;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import org.eclipse.gmf.runtime.notation.View;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.widgets.Display;
import org.fritzing.fritzing.diagram.edit.policies.PotentiometerCanonicalEditPolicy;
import org.fritzing.fritzing.diagram.edit.policies.PotentiometerItemSemanticEditPolicy;
import org.fritzing.fritzing.diagram.part.FritzingVisualIDRegistry;
import org.eclipse.gmf.runtime.notation.impl.NodeImpl;
import org.eclipse.emf.ecore.EObject;
import org.fritzing.fritzing.Terminal;

/**
 * @generated NOT
 */
public class PotentiometerEditPart extends PartEditPart {

	/**
	 * @generated
	 */
	public static final int VISUAL_ID = 2006;

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
	public PotentiometerEditPart(View view) {
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
				new PotentiometerItemSemanticEditPolicy());
		installEditPolicy(EditPolicyRoles.DRAG_DROP_ROLE,
				new DragDropEditPolicy());
		installEditPolicy(EditPolicyRoles.CANONICAL_ROLE,
				new PotentiometerCanonicalEditPolicy());
		installEditPolicy(EditPolicy.LAYOUT_ROLE, createLayoutEditPolicy());
		// XXX need an SCR to runtime to have another abstract superclass that would let children add reasonable editpolicies
		// removeEditPolicy(org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles.CONNECTION_HANDLES_ROLE);
	}

	/**
	 * @generated
	 */
	protected LayoutEditPolicy createLayoutEditPolicy() {
		LayoutEditPolicy lep = new LayoutEditPolicy() {

			protected EditPolicy createChildEditPolicy(EditPart child) {
				if (child instanceof IBorderItemEditPart) {
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

	/**
	 * @generated
	 */
	protected IFigure createNodeShape() {
		PotentiometerFigure figure = new PotentiometerFigure();
		return primaryShape = figure;
	}

	/**
	 * @generated
	 */
	public PotentiometerFigure getPrimaryShape() {
		return (PotentiometerFigure) primaryShape;
	}

	/**
	 * @generated NOT
	 */
	protected boolean addFixedChild(EditPart childEditPart) {
		if (childEditPart instanceof PotentiometerNameEditPart) {
			((PotentiometerNameEditPart) childEditPart)
					.setLabel(getPrimaryShape()
							.getFigurePotentiometerNameFigure());
			return true;
		}
		if (childEditPart instanceof Terminal2EditPart) {
			
			int positionConstants = PositionConstants.NONE;
			Object model = childEditPart.getModel();
			if (model instanceof NodeImpl) {
				EObject eobject = ((NodeImpl) model).getElement();
				if (eobject instanceof Terminal) {
					String name = ((Terminal) eobject).getName();
					if (name == null) {					
					}
					else if (name.equalsIgnoreCase("W")) {
						positionConstants = PositionConstants.SOUTH;
					}
					else if (name.equalsIgnoreCase("T1")) {
						positionConstants = PositionConstants.EAST;
					}
					else if (name.equalsIgnoreCase("T2")) {
						positionConstants = PositionConstants.WEST;
					}
					
				}
				
			}
			BorderItemLocator locator = new BorderItemLocator(getMainFigure(),
					positionConstants);
			getBorderedFigure().getBorderItemContainer().add(
					((Terminal2EditPart) childEditPart).getFigure(), locator);
			return true;
		}
		return false;
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
				.getType(PotentiometerNameEditPart.VISUAL_ID));
	}

	/**
	 * @generated
	 */
	public class PotentiometerFigure extends Ellipse {

		/**
		 * @generated
		 */
		private WrapLabel fFigurePotentiometerNameFigure;

		/**
		 * @generated
		 */
		public PotentiometerFigure() {
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

			Ellipse potentiometerStickFigure0 = new Ellipse();
			potentiometerStickFigure0
					.setBackgroundColor(POTENTIOMETERSTICKFIGURE0_BACK);
			potentiometerStickFigure0.setMaximumSize(new Dimension(getMapMode()
					.DPtoLP(14), getMapMode().DPtoLP(14)));
			potentiometerStickFigure0.setLocation(new Point(getMapMode()
					.DPtoLP(3), getMapMode().DPtoLP(3)));

			this.add(potentiometerStickFigure0);

			fFigurePotentiometerNameFigure = new WrapLabel();
			fFigurePotentiometerNameFigure.setText("..");

			fFigurePotentiometerNameFigure
					.setFont(FFIGUREPOTENTIOMETERNAMEFIGURE_FONT);

			this.add(fFigurePotentiometerNameFigure);

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
		public WrapLabel getFigurePotentiometerNameFigure() {
			return fFigurePotentiometerNameFigure;
		}

	}

	/**
	 * @generated
	 */
	static final Color THIS_FORE = new Color(null, 0, 0, 0);

	/**
	 * @generated
	 */
	static final Color THIS_BACK = new Color(null, 0, 0, 0);

	/**
	 * @generated
	 */
	static final Color POTENTIOMETERSTICKFIGURE0_BACK = new Color(null, 150,
			150, 150);

	/**
	 * @generated
	 */
	static final Font FFIGUREPOTENTIOMETERNAMEFIGURE_FONT = new Font(Display
			.getCurrent(),
			Display.getDefault().getSystemFont().getFontData()[0].getName(), 8,
			SWT.NORMAL);

}
