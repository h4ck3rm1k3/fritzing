/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;

import java.util.Iterator;

import org.eclipse.draw2d.ConnectionAnchor;
import org.eclipse.draw2d.Ellipse;
import org.eclipse.draw2d.Graphics;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.RectangleFigure;
import org.eclipse.draw2d.Shape;
import org.eclipse.draw2d.StackLayout;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Insets;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.draw2d.geometry.PrecisionPoint;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.gef.EditPart;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.Request;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.editpolicies.LayoutEditPolicy;
import org.eclipse.gef.editpolicies.NonResizableEditPolicy;
import org.eclipse.gef.editpolicies.ResizableEditPolicy;
import org.eclipse.gef.handles.HandleBounds;
import org.eclipse.gef.requests.CreateRequest;
import org.eclipse.gef.requests.DropRequest;
import org.eclipse.gef.requests.ReconnectRequest;
import org.eclipse.gmf.runtime.diagram.ui.editparts.BorderedBorderItemEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IBorderItemEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.BorderItemSelectionEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.ComponentEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.GraphicalNodeEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.figures.BorderItemLocator;
import org.eclipse.gmf.runtime.diagram.ui.requests.CreateConnectionViewRequest;
import org.eclipse.gmf.runtime.diagram.ui.requests.CreateUnspecifiedTypeConnectionRequest;
import org.eclipse.gmf.runtime.draw2d.ui.internal.figures.TransparentBorder;
import org.eclipse.gmf.runtime.gef.ui.figures.DefaultSizeNodeFigure;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import org.eclipse.gmf.runtime.gef.ui.figures.SlidableAnchor;
import org.eclipse.gmf.runtime.notation.View;
import org.eclipse.swt.graphics.Color;
import org.fritzing.fritzing.diagram.edit.parts.SketchEditPart.TerminalAnchor;
import org.fritzing.fritzing.diagram.edit.policies.Terminal2ItemSemanticEditPolicy;
import org.fritzing.fritzing.diagram.part.FritzingVisualIDRegistry;
import org.fritzing.fritzing.diagram.edit.policies.NonDeleteComponentEditPolicy;
import org.eclipse.swt.graphics.Color;

/**
 * @generated
 */
public class Terminal2EditPart extends BorderedBorderItemEditPart {

	/**
	 * @generated
	 */
	public static final int VISUAL_ID = 3001;

	/**
	 * @generated
	 */
	protected IFigure contentPane;

	/**
	 * @generated NOT
	 */
	public static final int standardPlateMeasure = 10;

	/**
	 * @generated NOT
	 */
	public static final int standardTerminalMeasure = 5;
	
	public static final int standardFeedbackInset = -1;


	/**
	 * @generated
	 */
	protected IFigure primaryShape;

	protected ConnectionAnchor legConnectionAnchor;

	/**
	 * @generated
	 */
	public Terminal2EditPart(View view) {
		super(view);
	}

	public Point getLegTargetPosition() {
		if (this.getParent() instanceof PartEditPart) {
			return ((PartEditPart) this.getParent()).getLegTargetPosition(this);
		}

		return null;
	}

	public void displayTargetFeedback(boolean display) {
		((TerminalDefaultSizeNodeFigure) this.getMainFigure())
				.displayFeedback(display);
	}

	public ConnectionAnchor getTargetConnectionAnchor(Request request) {
		return super.getTargetConnectionAnchor(request);
	}

	/**
	 * @generated NOT
	 */
	protected void createDefaultEditPolicies() {

		super.createDefaultEditPolicies();
		installEditPolicy(EditPolicy.PRIMARY_DRAG_ROLE,
				getPrimaryDragEditPolicy());
		installEditPolicy(EditPolicyRoles.SEMANTIC_ROLE,
				new Terminal2ItemSemanticEditPolicy());
		installEditPolicy(EditPolicy.LAYOUT_ROLE, createLayoutEditPolicy());

		// don't want delete
		installEditPolicy(EditPolicy.COMPONENT_ROLE,
				new NonDeleteComponentEditPolicy());

		installEditPolicy(EditPolicy.GRAPHICAL_NODE_ROLE,
				new Terminal2GraphicalNodeEditPolicy());

		// make it non-selectable? (doesn't seem to work)
		//removeEditPolicy(EditPolicyRoles.CONNECTION_HANDLES_ROLE);
		//removeEditPolicy(EditPolicy.PRIMARY_DRAG_ROLE);

		// POPUP_BAR and CONNECTOR_HANDLES are disabled in preferences

		// XXX need an SCR to runtime to have another abstract superclass that would let children add reasonable editpolicies
		// removeEditPolicy(org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles.CONNECTION_HANDLES_ROLE);
	}

	/**
	 * @generated NOT
	 */
	public void setTextColor(Color color) {
		for (int i = 0; i < this.getChildren().size(); i++) {
			if (this.getChildren().get(i) instanceof TerminalName2EditPart) {
				((TerminalName2EditPart) this.getChildren().get(i))
						.setFontColorEx(color);
				break;
			}
		}

	}

	/**
	 * @generated NOT
	 */
	protected LayoutEditPolicy createLayoutEditPolicy() {
		LayoutEditPolicy lep = new LayoutEditPolicy() {

			protected EditPolicy createChildEditPolicy(EditPart child) {
				if (child instanceof IBorderItemEditPart) {
					BorderItemSelectionEditPolicy bisep = new BorderItemSelectionEditPolicy();
					bisep.setDragAllowed(false); // disable move
					return bisep;
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
	 * @generated NOT
	 */
	protected IFigure createNodeShape() {
		TerminalFigure figure = new TerminalFigure(this);
		return primaryShape = figure;
	}

	/**
	 * @generated
	 */
	public TerminalFigure getPrimaryShape() {
		return (TerminalFigure) primaryShape;
	}

	/**
	 * @generated NOT
	 */
	protected void addBorderItem(IFigure borderItemContainer,
			IBorderItemEditPart borderItemEditPart) {
		if (borderItemEditPart instanceof TerminalName2EditPart) {
			BorderItemLocator locator = new BorderItemLocator(getMainFigure(),
					PositionConstants.SOUTH_WEST);
			locator.setBorderItemOffset(new Dimension(0, 0));
			borderItemContainer.add(borderItemEditPart.getFigure(), locator);
		} else {
			super.addBorderItem(borderItemContainer, borderItemEditPart);
		}
	}

	/**
	 * @generated NOT
	 */
	protected NodeFigure createNodePlate() {
		DefaultSizeNodeFigure result = new TerminalDefaultSizeNodeFigure(
				getMapMode().DPtoLP(standardPlateMeasure), getMapMode().DPtoLP(
						standardPlateMeasure));

		//FIXME: workaround for #154536
		result.getBounds().setSize(result.getPreferredSize());
		return result;
	}

	/**
	 * @generated NOT
	 */
	public EditPolicy getPrimaryDragEditPolicy() {

		EditPolicy result = super.getPrimaryDragEditPolicy();
		if (result instanceof ResizableEditPolicy) {
			ResizableEditPolicy ep = (ResizableEditPolicy) result;
			ep.setResizeDirections(PositionConstants.NONE);
		}
		if (result instanceof NonResizableEditPolicy) {
			// don't allow terminals to be moved
			((NonResizableEditPolicy) result).setDragAllowed(false);
		}
		return result;

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
				.getType(TerminalName2EditPart.VISUAL_ID));
	}

	void setLegConnectionAnchor(ConnectionAnchor ca) {
		legConnectionAnchor = ca;
	}

	ConnectionAnchor getLegConnectionAnchor() {
		return legConnectionAnchor;
	}

	public boolean hasLeg() {
		for (Iterator it = this.getSourceConnections().iterator(); it.hasNext();) {
			Object o = it.next();
			if (o instanceof LegEditPart)
				return true;
		}

		return false;
	}

	public boolean isFemale() {
		if (this.getParent() instanceof PartEditPart) {
			return ((PartEditPart) this.getParent()).isTerminalFemale(this);

		}

		return false;
	}

	public class Terminal2GraphicalNodeEditPolicy extends
			GraphicalNodeEditPolicy {
		protected void showTargetConnectionFeedback(DropRequest request) {
			xTargetConnectionFeedback(request, true);
		}

		protected void eraseTargetConnectionFeedback(DropRequest request) {
			xTargetConnectionFeedback(request, false);
		}

		protected void xTargetConnectionFeedback(DropRequest request,
				boolean display) {
			if (request instanceof ReconnectRequest) {
				EditPart target = ((ReconnectRequest) request).getTarget();
				if (target instanceof Terminal2EditPart) {
					((Terminal2EditPart) target).displayTargetFeedback(display);
				}
			} else if (request instanceof CreateUnspecifiedTypeConnectionRequest) {
				EditPart target = ((CreateUnspecifiedTypeConnectionRequest) request)
						.getTargetEditPart();
				if (target instanceof Terminal2EditPart) {
					((Terminal2EditPart) target).displayTargetFeedback(display);
				}
			}
		}
	}

	public class TerminalDefaultSizeNodeFigure extends DefaultSizeNodeFigure {
		protected boolean displayFeedbackFlag;
		protected int standardFeedbackInsetConverted = getMapMode().DPtoLP(standardFeedbackInset);

		public TerminalDefaultSizeNodeFigure(Dimension defSize) {
			super(defSize);
			displayFeedbackFlag = false;
		}

		public TerminalDefaultSizeNodeFigure(int width, int height) {
			super(width, height);
		}

		protected ConnectionAnchor createDefaultAnchor() {
			return new TerminalSlidableAnchor(this);
		}

		public ConnectionAnchor getTargetConnectionAnchorAt(Point p) {
			return createDefaultAnchor();
		}

		public void displayFeedback(boolean display) {
			if (display != displayFeedbackFlag) {
				displayFeedbackFlag = display;
				if (display) {
					this.setBackgroundColor(THIS_FEED);
				} else {
					this.setBackgroundColor(THIS_BACK);
				}
				this.invalidate();
			}
		}

		protected void paintFigure(Graphics graphics) {
			if (displayFeedbackFlag) {
				Rectangle tempRect = new Rectangle(getBounds());
				tempRect.expand(standardFeedbackInsetConverted, standardFeedbackInsetConverted);
				graphics.fillRectangle(tempRect);
			}

			super.paintFigure(graphics);
		}

	}

	public class TerminalSlidableAnchor extends SlidableAnchor {
		public TerminalSlidableAnchor(IFigure f) {
			super(f);
		}

		protected Rectangle getBox() {
			Rectangle rBox = getOwner().getBounds().getCopy();
			if (getOwner() instanceof HandleBounds)
				rBox = ((HandleBounds) getOwner()).getHandleBounds().getCopy();

			getOwner().translateToAbsolute(rBox);

			rBox.x += rBox.width / 2;
			rBox.y += rBox.height / 2;
			rBox.width = 0;
			rBox.height = 0;

			return rBox;
		}

	}

	/**
	 * @generated
	 */
	public class TerminalFigure extends RectangleFigure {

		/**
		 * @generated NOT
		 */
		public TerminalFigure() {
			// shouldn't be used!
		}

		protected int standardTerminalConverted;
		protected Terminal2EditPart terminalPart;

		/**
		 * @generated NOT
		 */
		public TerminalFigure(Terminal2EditPart terminalPart) {
			this.terminalPart = terminalPart;
			standardTerminalConverted = getMapMode().DPtoLP(
					standardTerminalMeasure);
			this.setLineWidth(0);
			this.setForegroundColor(THIS_FORE);
			this.setBackgroundColor(THIS_BACK);
			this.setPreferredSize(new Dimension(standardTerminalConverted,
					standardTerminalConverted));
		}

		public void paint(Graphics graphics) {
			if (terminalPart.isFemale()) {
				this.setVisible(false);
				return;
			}

			super.paint(graphics);
		}

		public void paintFigure(Graphics graphics) {
			if (!terminalPart.hasLeg()) {
				super.paintFigure(graphics);
			}
		}

		/**
		 * @generated NOT
		 */
		protected void fillShape(Graphics graphics) {
			Rectangle r = getBounds();
			int d = (r.width - standardTerminalConverted) / 2;
			r.x += d;
			r.y += d;
			r.setSize(standardTerminalConverted, standardTerminalConverted);
			graphics.fillRectangle(r);
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

	}

	/**
	 * @generated
	 */
	static final Color THIS_FORE = new Color(null, 80, 80, 80);

	/**
	 * @generated
	 */
	static final Color THIS_BACK = new Color(null, 0, 0, 0);

	static final Color THIS_FEED = new Color(null, 255, 0, 0);

}
