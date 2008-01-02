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
import org.fritzing.fritzing.GenericPart;
import org.fritzing.fritzing.diagram.edit.PartLoader;
import org.fritzing.fritzing.diagram.edit.PartLoaderRegistry;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;

/**
 * @generated
 */
public class GenericPartCreateCommand extends CreateElementCommand {
	/**
	 * @generated NOT
	 */
	
	PartLoader partLoader;

	/**
	 * @generated
	 */
	public GenericPartCreateCommand(CreateElementRequest req) {
		super(req);
		partLoader = (PartLoader) req.getParameter("partLoader");
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
	 * @generated
	 */
	protected EObject doDefaultElementCreation() {
		GenericPart newElement = (GenericPart) super.doDefaultElementCreation();
		if (newElement != null) {
			FritzingElementTypes.Initializers.GenericPart_2011.init(newElement);
		}
		
		if (partLoader == null) {
			// signal the user that something is wrong
			return newElement;		
		}
		
		// use "our" initializers instead		
		partLoader.createTerminals(newElement);	
		newElement.setSpecies(partLoader.getSpecies());
		newElement.setGenus(partLoader.getGenus());

		return newElement;
	}
}
