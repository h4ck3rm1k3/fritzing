/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.policies;

import org.eclipse.gef.EditPart;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.commands.UnexecutableCommand;
import org.eclipse.gmf.runtime.emf.type.core.commands.DestroyElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateRelationshipRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.DestroyElementRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.ReorientRelationshipRequest;
import org.eclipse.gmf.runtime.notation.impl.NodeImpl;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.diagram.edit.commands.LegCreateCommand;
import org.fritzing.fritzing.diagram.edit.commands.LegReorientCommand;
import org.fritzing.fritzing.diagram.edit.commands.WireCreateCommand;
import org.fritzing.fritzing.diagram.edit.commands.WireReorientCommand;
import org.fritzing.fritzing.diagram.edit.parts.LegEditPart;
import org.fritzing.fritzing.diagram.edit.parts.Terminal2EditPart;
import org.fritzing.fritzing.diagram.edit.parts.WireEditPart;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;

/**
 * @generated
 */
public class LegItemSemanticEditPolicy extends
		FritzingBaseItemSemanticEditPolicy {

	/**
	 * @generated
	 */
	protected Command getDestroyElementCommand(DestroyElementRequest req) {
		return getGEFWrapper(new DestroyElementCommand(req));
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
		case LegEditPart.VISUAL_ID:
			return getGEFWrapper(new LegReorientCommand(req));
		}
		return super.getReorientRelationshipCommand(req);
	}

}
