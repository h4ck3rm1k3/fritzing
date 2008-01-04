package org.fritzing.fritzing.diagram.edit.commands;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gmf.runtime.emf.type.core.commands.CreateElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateElementRequest;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.GenericPart;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.diagram.edit.PartLoader;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;

/**
 * @generated NOT
 */
public class PartCreateCommand extends CreateElementCommand {
	
	/**
	 * @generated NOT
	 */	
	PartLoader partLoader;

	/**
	 * @generated NOT
	 */
	public PartCreateCommand(CreateElementRequest request) {
		super(request);
		partLoader = (PartLoader) request.getParameter("partLoader");		
	}
	
	/**
	 * @generated NOT
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
	 * @generated NOT
	 */
	protected EClass getEClassToEdit() {
		return FritzingPackage.eINSTANCE.getComposite();
	}

	/**
	 * @generated NOT
	 */
	protected EObject doDefaultElementCreation() {
		try {
			Part newElement = (Part) super.doDefaultElementCreation();
			if (newElement == null) {
				// signal the user that something is wrong
				return null;
			}
			
			if (partLoader == null) {
				// signal the user that something is wrong
				return newElement;		
			}
			
			// use "our" initializers instead		
			partLoader.initialize(newElement);	
	
			return newElement;
		}
		catch (Exception ex) {
			// alert the user
			return null;
		}
	}
	
}
