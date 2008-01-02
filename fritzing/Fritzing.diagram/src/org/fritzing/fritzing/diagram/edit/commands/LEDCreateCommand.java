/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.commands;

import java.util.Collection;
import java.util.Iterator;

import org.eclipse.emf.common.util.EList;
import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.EStructuralFeature;
import org.eclipse.gef.EditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.DiagramEditPart;
import org.eclipse.gmf.runtime.emf.type.core.commands.CreateElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateElementRequest;
import org.eclipse.gmf.runtime.notation.View; //import org.fritzing.fritzing.Button;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.Wire;
import org.fritzing.fritzing.diagram.edit.PartLoader;
import org.fritzing.fritzing.diagram.edit.PartLoaderRegistry;
import org.fritzing.fritzing.diagram.edit.parts.SketchEditPart;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditor;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorUtil;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;
import org.fritzing.fritzing.impl.SketchImpl;

/**
 * @generated
 */
public class LEDCreateCommand extends CreateElementCommand {

	/**
	 * @generated
	 */
	public LEDCreateCommand(CreateElementRequest req) {
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
		org.fritzing.fritzing.LED newElement = (org.fritzing.fritzing.LED) super
				.doDefaultElementCreation();
		if (newElement != null) {
			FritzingElementTypes.Initializers.LED_2002.init(newElement);
		}

		// use "our" initializers instead		
		PartLoader partLoader = PartLoaderRegistry.getInstance().get("libraries/core/led/partdescription.xml");
		partLoader.createTerminals(newElement);		

		
		/*

 		// attempt to create a wire
 		// it doesn't appear on the diagram
 		// but if you save and reload, it shows up
 
		EObject wire = FritzingPackage.eINSTANCE.getWire()
		.getEPackage().getEFactoryInstance().create(
				FritzingPackage.eINSTANCE.getWire());
				
		EList<Terminal> trmnls = newElement.getTerminals();

		if (trmnls.size() > 1) {
			((Wire) wire).setSource(trmnls.get(0));
			((Wire) wire).setTarget(trmnls.get(1));
			newElement.getParent().getWires().add((Wire) wire);
			 
			
//			//((SketchImpl) newElement.getParent()).
//			
//			FritzingDiagramEditor diagram = FritzingDiagramEditorUtil.getActiveDiagramPart();
//			if (diagram != null) {
//				DiagramEditPart part = diagram.getDiagramEditPart();
//				//((SketchEditPart) part).refresh();
//				
//				for (Iterator it = part.getChildren().iterator(); it.hasNext(); ) {
//					EditPart editPart = (EditPart) it.next();
//					if (editPart.getModel() == newElement) {
//						System.out.println("got part");
//					}
//				}
//				
//				//refreshSourceConnections()
//				//refreshTargetConnections()
//			}
		
		}
		
		*/
	
		return newElement;
	}
}
