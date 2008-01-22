/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;


import java.util.List;

import org.eclipse.draw2d.ConnectionAnchor;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.StackLayout;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.emf.common.notify.Notification;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.impl.EAttributeImpl;
import org.eclipse.gef.ConnectionEditPart;
import org.eclipse.gef.EditPart;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.Request;
import org.eclipse.gef.RequestConstants;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.editparts.AbstractEditPart;
import org.eclipse.gef.editparts.ZoomListener;
import org.eclipse.gef.editpolicies.LayoutEditPolicy;
import org.eclipse.gef.editpolicies.NonResizableEditPolicy;
import org.eclipse.gef.requests.CreateRequest;
import org.eclipse.gmf.runtime.diagram.core.util.ViewUtil;
import org.eclipse.gmf.runtime.diagram.ui.editparts.AbstractBorderedShapeEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IBorderItemEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IGraphicalEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IRotatableEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.BorderItemSelectionEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.figures.BorderItemLocator;
import org.eclipse.gmf.runtime.diagram.ui.figures.IBorderItemLocator;
import org.eclipse.gmf.runtime.diagram.ui.parts.DiagramCommandStack;
import org.eclipse.gmf.runtime.diagram.ui.requests.CreateConnectionViewRequest;
import org.eclipse.gmf.runtime.draw2d.ui.figures.ConstrainedToolbarLayout;
import org.eclipse.gmf.runtime.gef.ui.figures.DefaultSizeNodeFigure;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import org.eclipse.gmf.runtime.notation.View;
import org.eclipse.gmf.runtime.notation.impl.DiagramImpl;
import org.eclipse.gmf.runtime.notation.impl.NodeImpl;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.Sketch;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.diagram.edit.PartLoader;
import org.fritzing.fritzing.diagram.edit.PartLoaderRegistry;
import org.fritzing.fritzing.diagram.edit.policies.RotatableNonresizableShapeEditPolicy;
import org.fritzing.fritzing.diagram.part.FritzingLinkDescriptor;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;

/**
 * @generated NOT
 */
public class PartEditPart extends AbstractBorderedShapeEditPart implements IRotatableEditPart
{

	PartLoader partLoader;
	
	protected Point gridOffset;
	
	protected boolean legsInitialized;
	
	/**
	 * @generated NOT
	 */
	protected IFigure contentPane;

	/**
	 * @generated NOT
	 */
	protected IFigure primaryShape;
	
	/**
	 * @generated NOT
	 */
	public PartEditPart(View view) {
		super(view);
		legsInitialized = false;
		EObject element = view.getElement();
		if (element instanceof Part) {
			String genus = ((Part) element).getGenus();
			String species = ((Part) element).getSpecies();
			if (genus != null && species != null) {
				partLoader = PartLoaderRegistry.getInstance().get(
						genus + species);
			}
		}
	}
		
	public int DPtoLP(int deviceUnit) {
		return getMapMode().DPtoLP(deviceUnit);		
	}
	
	public PartLoader getPartLoader() {
		return partLoader;
	}
	
	public ConnectionAnchor getTargetConnectionAnchor(ConnectionEditPart connection) {
		EditPart part = connection.getSource();
		if (part instanceof Terminal2EditPart) {
			// if it's null, create one here
			return ((Terminal2EditPart) part).getLegConnectionAnchor();
		}
		return null;
	}
	
	public Point getLegTargetPosition(Terminal2EditPart terminalPart) {
		try {
			Terminal terminal = (Terminal) ((NodeImpl) terminalPart.getModel()).getElement();
			Point p = partLoader.getTerminalLegTargetPosition(terminal.getId());
			p.x = getMapMode().LPtoDP(p.x);
			p.y = getMapMode().LPtoDP(p.y);
			return p;
		}
		catch (Exception ex) {
			
		}			
		
		return null;
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

		addZoomListener(); // needed for zoom-dependent figure
		
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
	
	

	/**
	 * Creates figure for this edit part.
	 * 
	 * Body of this method does not depend on settings in generation model
	 * so you may safely remove <i>generated</i> tag and modify it.
	 * 
	 * @generated NOT
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
	 * @generated NOT
	 */
	protected IFigure createNodeShape() {
		PartFigure figure = new PartFigure(partLoader);
		return primaryShape = figure;
	}
	
	/**
	 * Default implementation treats passed figure as content pane.
	 * Respects layout one may have set for generated figure.
	 * @param nodeShape instance of generated figure class
	 * @generated NOT
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
	 * @generated NOT
	 */
	public IFigure getContentPane() {
		if (contentPane != null) {
			return contentPane;
		}
		return super.getContentPane();
	}
	
	/**
	 * @generated NOT
	 */
	protected NodeFigure createNodePlate() {
		Dimension size = partLoader.getSize();
//		DefaultSizeNodeFigure result = new DefaultSizeNodeFigure(getMapMode()
//				.DPtoLP((int) (size.width / multiplier)), getMapMode().DPtoLP(
//				(int) (size.height / multiplier)));
		DefaultSizeNodeFigure result = new DefaultSizeNodeFigure(size.width, size.height);
		return result;
	}

	/**
	 * @generated NOT
	 */
	public PartFigure getPrimaryShape() {
		return (PartFigure) primaryShape;
	}

	protected void addZoomListener() {
		((FritzingDiagramRootEditPart)getRoot()).getZoomManager().addZoomListener(
			new ZoomListener() {
				public void zoomChanged(double zoom) {
					((PartFigure)getPrimaryShape()).setZoom(zoom);
				}
			});
		
		getPrimaryShape().setZoom(
				((FritzingDiagramRootEditPart)getRoot()).getZoomManager().getZoom());
	}
   
	protected void handleNotificationEvent(Notification notification) {
		Object feature = notification.getFeature();		        
		if (feature instanceof EAttributeImpl) {
			// XXX: initializeLegs fails if we trigger it too soon after part creation
			// it seems to work at this point
			
			// there is probably a better place to call initializeLegs:
			// the model already has a leg, and initializeLegs creates a view
			// the problem is that creating the view doesn't seem to work
			// if we call it too soon after the EditPart is created.
			// waiting for the first feature notification is a hack
			// though it seems to work...
			// what about a signal when the PartFigure is created?
			if (!legsInitialized) {
				legsInitialized = true;
 				initializeLegs();
			}
		}
		super.handleNotificationEvent(notification);
	}
	
	public boolean isTerminalFemale(Terminal2EditPart terminal) {
		if (partLoader == null) return false;
		
		Object o = terminal.getModel();
		if (!(o instanceof NodeImpl)) return false;
		
		EObject e = ((NodeImpl) o).getElement();
		if (e == null) return false;
			
		return partLoader.getTerminalFemale(((Terminal) e).getId());
	}
	
	void initializeLegs() {
		legsInitialized = true;
				
		try {
			List<AbstractEditPart> chldrn = this.getChildren();
			Sketch sketch = (Sketch) ((DiagramImpl) this.getParent().getModel()).getElement();
									
			for (int i = 0; i < chldrn.size(); i++) {				
				if (chldrn.get(i) instanceof Terminal2EditPart) {
					Terminal2EditPart child = (Terminal2EditPart) chldrn.get(i);
					Terminal terminal = (Terminal) ((NodeImpl) child.getModel()).getElement();
					String s = partLoader.getTerminalType(terminal.getId());
										
					if (!s.equalsIgnoreCase("leg")) continue;
					
					if (child.hasLeg()) {
						// already has a leg part
						continue;
					}
																				
					FritzingLinkDescriptor fld = new FritzingLinkDescriptor(terminal, 
							sketch,
							terminal.getLeg(), FritzingElementTypes.Leg_4003, LegEditPart.VISUAL_ID);
									
					CreateConnectionViewRequest.ConnectionViewDescriptor descriptor = 
						new CreateConnectionViewRequest.ConnectionViewDescriptor(
							fld.getSemanticAdapter(), String.valueOf(LegEditPart.VISUAL_ID),
							ViewUtil.APPEND, true, ((IGraphicalEditPart) this.getParent())
									.getDiagramPreferencesHint());
					CreateConnectionViewRequest ccr = new CreateConnectionViewRequest(
							descriptor);
					ccr.setType(RequestConstants.REQ_CONNECTION_START);
					ccr.setSourceEditPart(child);
					Command cmd1 = child.getCommand(ccr);
									
					ccr.setType(RequestConstants.REQ_CONNECTION_END);
					ccr.setTargetEditPart(this.getParent());				
					Command cmd2 = this.getParent().getCommand(ccr);
							
					if (cmd2 != null && cmd2.canExecute()) {
						DiagramCommandStack commandStack = this.getDiagramEditDomain()
						.getDiagramCommandStack();
						commandStack.execute(cmd2);	
						
						// the new LegEditPart isn't actually created yet, even though the execute method has returned.
						// so there's no way to access the new LegEditPart from here
					}
					else {
						// alert the user?
					}
				}
			}
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	protected boolean addEastWestFixedChild(EditPart childEditPart) {
		if (childEditPart instanceof Terminal2EditPart) {
			int terminalPosition = PositionConstants.NONE;
			Point p = null;
			Object model = childEditPart.getModel();		
			if (model instanceof NodeImpl) {
				EObject eobject = ((NodeImpl) model).getElement();
				if (eobject instanceof Terminal) {
					String name = ((Terminal) eobject).getId();
					p = findTerminal(childEditPart);
					if (p == null) {
						terminalPosition = EastWestBorderItemLocator.parseTerminalName(name);
					}
				}
			}
			
			IBorderItemLocator locator = null;			
			if ((p != null) || (terminalPosition != PositionConstants.NONE)) {
				locator = new EastWestBorderItemLocator(getMainFigure(), this, terminalPosition, p);
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

	protected Point findTerminal(EditPart childEditPart) {
		if (!(childEditPart instanceof Terminal2EditPart)) return null;
		
		Object model = childEditPart.getModel();
	
		if (model instanceof NodeImpl) {
			EObject eobject = ((NodeImpl) model).getElement();
			if (eobject instanceof Terminal) {
				String name = ((Terminal) eobject).getId();
				if (name != null) {
					Point q = partLoader.getTerminalPoint(name);
					if (q != null) {
						return q;
					}
				}
			}
		}
		
		return null;
	}

}
