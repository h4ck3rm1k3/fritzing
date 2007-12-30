/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.commands;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gmf.runtime.emf.type.core.commands.CreateElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateElementRequest;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.Resistor;
import org.fritzing.fritzing.diagram.edit.PartLoader;
import org.fritzing.fritzing.diagram.edit.PartLoaderRegistry;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;

/**
 * @generated
 */
public class ResistorCreateCommand extends CreateElementCommand {

	/**
	 * @generated
	 */
	public ResistorCreateCommand(CreateElementRequest req) {
		super(req);
	}

	/**
	 * @generated
	 */
	protected EObject getElementToEdit() {
		EObject container = ((CreateElementRequest) getRequest())
				.getContainer();
		if (container instanceof View) {
			container = ((View) container).getElement();
		}
		return container;
	}

	/**
	 * @generated
	 */
	protected EClass getEClassToEdit() {
		return FritzingPackage.eINSTANCE.getComposite();
	}

	/**
	 * @generated NOT
	 */
	protected EObject doDefaultElementCreation() {
		Resistor newElement = (Resistor) super.doDefaultElementCreation();
		if (newElement != null) {
			FritzingElementTypes.Initializers.Resistor_2003.init(newElement);
		}
		
		// use "our" initializers instead		
		PartLoader partLoader = PartLoaderRegistry.getInstance().get("libraries/core/resistor/partdescription.xml");
		partLoader.createTerminals(newElement);		

		return newElement;
	}
}
