/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.edit.policies;

import java.util.Iterator;

import org.eclipse.emf.ecore.EAnnotation;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.commands.CompoundCommand;
import org.eclipse.gmf.runtime.emf.type.core.commands.DestroyElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateElementRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateRelationshipRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.DestroyElementRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.ReorientRelationshipRequest;
import org.eclipse.gmf.runtime.notation.Node;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.pcb.edit.commands.Terminal2CreateCommand;
import org.fritzing.pcb.edit.commands.WireCreateCommand;
import org.fritzing.pcb.edit.commands.WireReorientCommand;
import org.fritzing.pcb.edit.parts.Terminal2EditPart;
import org.fritzing.pcb.edit.parts.WireEditPart;
import org.fritzing.pcb.part.FritzingVisualIDRegistry;
import org.fritzing.pcb.providers.FritzingElementTypes;

/**
 * @generated NOT
 */
public class ResistorItemSemanticEditPolicy extends
		PartItemSemanticEditPolicy {

	/**
	 * @generated NOT
	 */
	protected Command getCreateCommand(CreateElementRequest req) {
		return super.getCreateCommand(req);
	}

	/**
	 * @generated NOT
	 */
	protected Command getDestroyElementCommand(DestroyElementRequest req) {
		return super.getDestroyElementCommand(req);	
	}

	/**
	 * @generated NOT
	 */
	protected void addDestroyChildNodesCommand(CompoundCommand cmd) {
		super.addDestroyChildNodesCommand(cmd);
	}

}
