/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.edit.policies;

import org.eclipse.gef.EditPart;
import org.eclipse.gef.commands.Command;
import org.eclipse.gmf.runtime.emf.type.core.commands.DestroyElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateRelationshipRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.DestroyElementRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.ReorientRelationshipRequest;
import org.eclipse.gmf.runtime.notation.impl.NodeImpl;
import org.fritzing.fritzing.Terminal;
import org.fritzing.pcb.edit.commands.LegCreateCommand;
import org.fritzing.pcb.edit.commands.LegReorientCommand;
import org.fritzing.pcb.edit.parts.LegEditPart;
import org.fritzing.pcb.edit.parts.WireEditPart;
import org.fritzing.pcb.providers.FritzingElementTypes;

/**
 * @generated
 */
public class WireItemSemanticEditPolicy extends
		FritzingBaseItemSemanticEditPolicy {

	/**
	 * @generated
	 */
	protected Command getDestroyElementCommand(DestroyElementRequest req) {
		return getGEFWrapper(new DestroyElementCommand(req));
	}

}
