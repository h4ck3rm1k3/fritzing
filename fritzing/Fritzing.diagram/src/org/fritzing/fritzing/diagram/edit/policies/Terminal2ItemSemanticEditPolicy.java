/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.policies;

import org.eclipse.gef.commands.Command;
import org.eclipse.gef.commands.CompoundCommand;
import org.eclipse.gmf.runtime.emf.type.core.commands.DestroyElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateRelationshipRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.DestroyElementRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.ReorientRelationshipRequest;
import org.fritzing.fritzing.diagram.edit.commands.WireCreateCommand;
import org.fritzing.fritzing.diagram.edit.commands.WireReorientCommand;
import org.fritzing.fritzing.diagram.edit.parts.WireEditPart;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;

/**
 * @generated
 */
public class Terminal2ItemSemanticEditPolicy extends
		FritzingBaseItemSemanticEditPolicy {

	/**
	 * @generated
	 */
	protected Command getDestroyElementCommand(DestroyElementRequest req) {
		// don't want to delete
		return null;
		
//		CompoundCommand cc = getDestroyEdgesCommand();
//		addDestroyShortcutsCommand(cc);
//		cc.add(getGEFWrapper(new DestroyElementCommand(req)));
//		return cc.unwrap();
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
		}
		return super.getReorientRelationshipCommand(req);
	}

}
