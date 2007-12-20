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
import org.fritzing.fritzing.diagram.expressions.FritzingAbstractExpression;
import org.fritzing.fritzing.diagram.expressions.FritzingOCLFactory;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorPlugin;
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

		try {
			// use "our" initializers instead
			URL url = FileLocator.find(FritzingDiagramEditorPlugin
					.getInstance().getBundle(), new Path(
					"icons/parts/arduinopartdescription.xml"), null);

			PartLoader partLoader = new PartLoader();
			partLoader.loadXML(FileLocator.toFileURL(url));
			for (Enumeration<String> e = partLoader.getTerminalKeys(); e
					.hasMoreElements();) {
				String name = e.nextElement();
				if (name == null || name == "")
					continue;

				EObject terminal = FritzingPackage.eINSTANCE.getTerminal()
						.getEPackage().getEFactoryInstance().create(
								FritzingPackage.eINSTANCE.getTerminal());

				EStructuralFeature feature = FritzingPackage.eINSTANCE
						.getPart_Terminals();
				((Collection) newElement.eGet(feature)).add(terminal);
				FritzingAbstractExpression expr = FritzingOCLFactory
						.getExpression("\'" + name + "\'",
								FritzingPackage.eINSTANCE.getTerminal());
				expr.assignTo(FritzingPackage.eINSTANCE.getTerminal_Name(),
						terminal);
			}

		} catch (Exception ex) {
			// how to alert the user?
		}

		return newElement;
	}
}
