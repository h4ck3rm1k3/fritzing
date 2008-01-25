/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.policies;

import java.util.Iterator;

import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.transaction.TransactionalEditingDomain;
import org.eclipse.gef.EditPart;
import org.eclipse.gef.Request;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.commands.CompoundCommand;
import org.eclipse.gmf.runtime.diagram.core.util.ViewUtil;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IGraphicalEditPart;
import org.eclipse.gmf.runtime.diagram.ui.requests.EditCommandRequestWrapper;
import org.eclipse.gmf.runtime.diagram.ui.requests.RequestConstants;
import org.eclipse.gmf.runtime.emf.type.core.commands.DestroyElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateRelationshipRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.DestroyElementRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.ReorientRelationshipRequest;
import org.eclipse.gmf.runtime.notation.Edge;
import org.eclipse.gmf.runtime.notation.View;
import org.eclipse.gmf.runtime.notation.impl.EdgeImpl;
import org.eclipse.gmf.runtime.notation.impl.NodeImpl;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.diagram.edit.commands.LegCreateCommand;
import org.fritzing.fritzing.diagram.edit.commands.LegReorientCommand;
import org.fritzing.fritzing.diagram.edit.commands.TrackCreateCommand;
import org.fritzing.fritzing.diagram.edit.commands.TrackReorientCommand;
import org.fritzing.fritzing.diagram.edit.commands.WireCreateCommand;
import org.fritzing.fritzing.diagram.edit.commands.WireReorientCommand;
import org.fritzing.fritzing.diagram.edit.parts.LegEditPart;
import org.fritzing.fritzing.diagram.edit.parts.Terminal2EditPart;
import org.fritzing.fritzing.diagram.edit.parts.TrackEditPart;
import org.fritzing.fritzing.diagram.edit.parts.WireEditPart;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;
import org.fritzing.fritzing.impl.LegImpl;

/**
 * @generated
 */
public class Terminal2ItemSemanticEditPolicy extends
		FritzingBaseItemSemanticEditPolicy {

	/**
	 * @generated
	 */
	protected Command getDestroyElementCommand(DestroyElementRequest req) {
		CompoundCommand cc = getDestroyEdgesCommand();
		addDestroyShortcutsCommand(cc);
		cc.add(getGEFWrapper(new DestroyElementCommand(req)));
		return cc.unwrap();
	}
	

	/*
	 * (non-Javadoc)
	 * @see org.fritzing.fritzing.diagram.edit.policies.FritzingBaseItemSemanticEditPolicy#getDestroyEdgesCommand()
	 */
	protected CompoundCommand getDestroyEdgesCommand() {				
		CompoundCommand cmd = new CompoundCommand();
		View view = (View) getHost().getModel();
		for (Iterator it = view.getSourceEdges().iterator(); it.hasNext();) {
			cmd.add(getDestroyElementCommand((Edge) it.next()));
		}
		for (Iterator it = view.getTargetEdges().iterator(); it.hasNext();) {
			Object obj = it.next();
			if ((obj instanceof EdgeImpl) && (((EdgeImpl) obj).getElement() instanceof LegImpl)) {
				// don't delete a leg connected to this terminal that actually belongs to another editpart
				continue;
			}
			
			cmd.add(getDestroyElementCommand((Edge) obj));
		}
		return cmd;
	}
	
	public boolean understandsRequest(Request request) {
		boolean result = super.understandsRequest(request);
												
		if ((result == true) && 
			(REQ_CONNECTION_START.equals(request.getType())
					|| REQ_CONNECTION_END.equals(request.getType())
					|| REQ_RECONNECT_SOURCE.equals(request.getType())
					|| REQ_RECONNECT_TARGET.equals(request.getType())
					|| REQ_SELECTION.equals(request.getType())
					|| REQ_MOVE.equals(request.getType())
				)
			) {
						
			// don't allow connections to a terminal that has a leg
			EditPart editPart = this.getHost();
			if (editPart instanceof Terminal2EditPart) {
				if (((Terminal2EditPart) editPart).hasLeg())
				{
					return false;
				}
			}			
		}
		
		return result;
	}


	/**
	 * @generated
	 */
	protected Command getCreateRelationshipCommand(CreateRelationshipRequest req) {
		Command command = req.getTarget() == null ? getStartCreateRelationshipCommand(req)
				: getCompleteCreateRelationshipCommand(req);
		return command != null ? command : super
				.getCreateRelationshipCommand(req);
	}

	/**
	 * @generated
	 */
	protected Command getStartCreateRelationshipCommand(
			CreateRelationshipRequest req) {
		if (FritzingElementTypes.Wire_4001 == req.getElementType()) {
			return getGEFWrapper(new WireCreateCommand(req, req.getSource(),
					req.getTarget()));
		}
		if (FritzingElementTypes.Track_4002 == req.getElementType()) {
			return getGEFWrapper(new TrackCreateCommand(req, req.getSource(),
					req.getTarget()));
		}
		if (FritzingElementTypes.Leg_4003 == req.getElementType()) {
			return getGEFWrapper(new LegCreateCommand(req, req.getSource(), req
					.getTarget()));
		}
		return null;
	}

	/**
	 * @generated
	 */
	protected Command getCompleteCreateRelationshipCommand(
			CreateRelationshipRequest req) {
		if (FritzingElementTypes.Wire_4001 == req.getElementType()) {
			return getGEFWrapper(new WireCreateCommand(req, req.getSource(),
					req.getTarget()));
		}
		if (FritzingElementTypes.Track_4002 == req.getElementType()) {
			return getGEFWrapper(new TrackCreateCommand(req, req.getSource(),
					req.getTarget()));
		}
		if (FritzingElementTypes.Leg_4003 == req.getElementType()) {
			return getGEFWrapper(new LegCreateCommand(req, req.getSource(), req
					.getTarget()));
		}
		return null;
	}

	/**
	 * Returns command to reorient EClass based link. New link target or source
	 * should be the domain model element associated with this node.
	 * 
	 * @generated
	 */
	protected Command getReorientRelationshipCommand(
			ReorientRelationshipRequest req) {
		switch (getVisualID(req)) {
		case WireEditPart.VISUAL_ID:
			return getGEFWrapper(new WireReorientCommand(req));
		case TrackEditPart.VISUAL_ID:
			return getGEFWrapper(new TrackReorientCommand(req));
		case LegEditPart.VISUAL_ID:
			return getGEFWrapper(new LegReorientCommand(req));
		}
		return super.getReorientRelationshipCommand(req);
	}

}
