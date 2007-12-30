/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.commands;

import java.net.URL;
import java.util.Collection;
import java.util.Enumeration;
import java.util.Hashtable;

import org.eclipse.core.runtime.FileLocator;
import org.eclipse.core.runtime.Path;
import org.eclipse.draw2d.geometry.Point;
import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.EStructuralFeature;
import org.eclipse.gmf.runtime.emf.type.core.commands.CreateElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateElementRequest;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.Arduino;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.diagram.edit.PartLoader;
import org.fritzing.fritzing.diagram.edit.PartLoaderRegistry;
import org.fritzing.fritzing.diagram.expressions.FritzingAbstractExpression;
import org.fritzing.fritzing.diagram.expressions.FritzingOCLFactory;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorPlugin;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorUtil;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;

/**
 * @generated
 */
public class ArduinoCreateCommand extends CreateElementCommand {

	/**
	 * @generated
	 */
	public ArduinoCreateCommand(CreateElementRequest req) {
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
		Arduino newElement = (Arduino) super.doDefaultElementCreation();
		if (newElement != null) {
			FritzingElementTypes.Initializers.Arduino_2001.init(newElement);
		}

		// use "our" initializers instead		
		PartLoader partLoader = PartLoaderRegistry.getInstance().get("libraries/core/arduino/partdescription.xml");
		partLoader.createTerminals(newElement);		

		return newElement;
	}
}
