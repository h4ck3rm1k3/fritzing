/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.edit.policies;

import org.eclipse.gef.commands.Command;
import org.eclipse.gef.commands.CompoundCommand;
import org.eclipse.gmf.runtime.emf.type.core.commands.DestroyElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateRelationshipRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.DestroyElementRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.ReorientRelationshipRequest;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.pcb.edit.commands.LegCreateCommand;
import org.fritzing.pcb.edit.commands.LegReorientCommand;
import org.fritzing.pcb.edit.commands.TrackCreateCommand;
import org.fritzing.pcb.edit.commands.TrackReorientCommand;
import org.fritzing.pcb.edit.commands.WireCreateCommand;
import org.fritzing.pcb.edit.commands.WireReorientCommand;
import org.fritzing.pcb.edit.parts.LegEditPart;
import org.fritzing.pcb.edit.parts.TrackEditPart;
import org.fritzing.pcb.edit.parts.WireEditPart;
import org.fritzing.pcb.providers.FritzingElementTypes;

/**
 * @generated
 */
public class TerminalItemSemanticEditPolicy extends
		FritzingBaseItemSemanticEditPolicy {

	/**
	 * @generated
	 */
	protected Command getDestroyElementCommand(DestroyElementRequest req) {
		CompoundCommand cc = getDestroyEdgesCommand();
		addDestroyShortcutsCommand(cc);
		View view = (View) getHost().getModel();
		if (view.getEAnnotation("Shortcut") != null) { //$NON-NLS-1$
			req.setElementToDestroy(view);
		}
		cc.add(getGEFWrapper(new DestroyElementCommand(req)));
		return cc.unwrap();
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
