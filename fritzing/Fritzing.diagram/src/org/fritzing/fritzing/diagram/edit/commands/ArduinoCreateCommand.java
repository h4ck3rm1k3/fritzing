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
public class ArduinoCreateCommand extends PartCreateCommand {

	/**
	 * @generated
	 */
	public ArduinoCreateCommand(CreateElementRequest req) {
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
