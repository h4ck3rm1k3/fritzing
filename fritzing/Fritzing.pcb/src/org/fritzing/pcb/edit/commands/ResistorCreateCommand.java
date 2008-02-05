/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.edit.commands;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gmf.runtime.emf.type.core.commands.CreateElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateElementRequest;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.Resistor;
import org.fritzing.pcb.edit.PartDefinitionRegistry;
import org.fritzing.pcb.edit.PartLoader;
import org.fritzing.pcb.providers.FritzingElementTypes;

/**
 * @generated NOT
 */
public class ResistorCreateCommand extends PartCreateCommand {

	/**
	 * @generated
	 */
	public ResistorCreateCommand(CreateElementRequest req) {
		super(req);
	}

	/**
	 * @generated NOT
	 */
	protected EObject getElementToEdit() {
		return super.getElementToEdit();
	}

	/**
	 * @generated NOT
	 */
	protected EClass getEClassToEdit() {
		return super.getEClassToEdit();
	}

	/**
	 * @generated NOT
	 */
	protected EObject doDefaultElementCreation() {
		return super.doDefaultElementCreation();

	}
}
