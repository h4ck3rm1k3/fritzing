package org.fritzing.fritzing.diagram.edit.commands;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gmf.runtime.emf.type.core.commands.CreateElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateElementRequest;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.diagram.edit.PartDefinition;
import org.fritzing.fritzing.diagram.edit.PartLoader;

public class PartCreateCommand extends CreateElementCommand {
	
	PartDefinition partDefinition;

	public PartCreateCommand(CreateElementRequest request) {
		super(request);
		partDefinition = (PartDefinition) request.getParameter(PartDefinition.REQUEST_PARAM);		
	}
	
	protected EObject getElementToEdit() {
		EObject container = ((CreateElementRequest) getRequest())
				.getContainer();
		if (container instanceof View) {
			container = ((View) container).getElement();
		}
		return container;
	}

	protected EClass getEClassToEdit() {
		return FritzingPackage.eINSTANCE.getComposite();
	}

	protected EObject doDefaultElementCreation() {
		try {
			Part newElement = (Part) super.doDefaultElementCreation();
			if (newElement == null) {
				// TODO: signal the user that something is wrong
				return null;
			}
			
			if (partDefinition == null) {
				// TODO: signal the user that something is wrong
				return newElement;		
			}
			
			// use "our" initializers instead		
			PartLoader.initialize(partDefinition, newElement);	
	
			return newElement;
		}
		catch (Exception ex) {
			// TODO: alert the user
			return null;
		}
	}
	
}
