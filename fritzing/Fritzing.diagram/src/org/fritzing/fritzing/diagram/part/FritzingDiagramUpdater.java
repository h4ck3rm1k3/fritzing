/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.part;

import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.EStructuralFeature;
import org.eclipse.emf.ecore.util.EcoreUtil;
import org.eclipse.gmf.runtime.notation.View;
import org.fritzing.fritzing.Arduino;
import org.fritzing.fritzing.Button;
import org.fritzing.fritzing.Composite;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.Resistor;
import org.fritzing.fritzing.Sketch;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.Wire;
import org.fritzing.fritzing.diagram.edit.parts.ArduinoEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ButtonEditPart;
import org.fritzing.fritzing.diagram.edit.parts.LEDEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ResistorEditPart;
import org.fritzing.fritzing.diagram.edit.parts.SketchEditPart;
import org.fritzing.fritzing.diagram.edit.parts.Terminal2EditPart;
import org.fritzing.fritzing.diagram.edit.parts.TerminalEditPart;
import org.fritzing.fritzing.diagram.edit.parts.WireEditPart;
import org.fritzing.fritzing.diagram.providers.FritzingElementTypes;

/**
 * @generated
 */
public class FritzingDiagramUpdater {

	/**
	 * @generated
	 */
	public static List getSemanticChildren(View view) {
		switch (FritzingVisualIDRegistry.getVisualID(view)) {
		case ArduinoEditPart.VISUAL_ID:
			return getArduino_2002SemanticChildren(view);
		case LEDEditPart.VISUAL_ID:
			return getLED_2004SemanticChildren(view);
		case ResistorEditPart.VISUAL_ID:
			return getResistor_2005SemanticChildren(view);
		case ButtonEditPart.VISUAL_ID:
			return getButton_2006SemanticChildren(view);
		case SketchEditPart.VISUAL_ID:
			return getSketch_1000SemanticChildren(view);
		}
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getArduino_2002SemanticChildren(View view) {
		if (!view.isSetElement()) {
			return Collections.EMPTY_LIST;
		}
		Arduino modelElement = (Arduino) view.getElement();
		List result = new LinkedList();
		for (Iterator it = modelElement.getTerminals().iterator(); it.hasNext();) {
			Terminal childElement = (Terminal) it.next();
			int visualID = FritzingVisualIDRegistry.getNodeVisualID(view,
					childElement);
			if (visualID == Terminal2EditPart.VISUAL_ID) {
				result.add(new FritzingNodeDescriptor(childElement, visualID));
				continue;
			}
		}
		return result;
	}

	/**
	 * @generated
	 */
	public static List getLED_2004SemanticChildren(View view) {
		if (!view.isSetElement()) {
			return Collections.EMPTY_LIST;
		}
		org.fritzing.fritzing.LED modelElement = (org.fritzing.fritzing.LED) view
				.getElement();
		List result = new LinkedList();
		for (Iterator it = modelElement.getTerminals().iterator(); it.hasNext();) {
			Terminal childElement = (Terminal) it.next();
			int visualID = FritzingVisualIDRegistry.getNodeVisualID(view,
					childElement);
			if (visualID == Terminal2EditPart.VISUAL_ID) {
				result.add(new FritzingNodeDescriptor(childElement, visualID));
				continue;
			}
		}
		return result;
	}

	/**
	 * @generated
	 */
	public static List getResistor_2005SemanticChildren(View view) {
		if (!view.isSetElement()) {
			return Collections.EMPTY_LIST;
		}
		Resistor modelElement = (Resistor) view.getElement();
		List result = new LinkedList();
		for (Iterator it = modelElement.getTerminals().iterator(); it.hasNext();) {
			Terminal childElement = (Terminal) it.next();
			int visualID = FritzingVisualIDRegistry.getNodeVisualID(view,
					childElement);
			if (visualID == Terminal2EditPart.VISUAL_ID) {
				result.add(new FritzingNodeDescriptor(childElement, visualID));
				continue;
			}
		}
		return result;
	}

	/**
	 * @generated
	 */
	public static List getButton_2006SemanticChildren(View view) {
		if (!view.isSetElement()) {
			return Collections.EMPTY_LIST;
		}
		Button modelElement = (Button) view.getElement();
		List result = new LinkedList();
		for (Iterator it = modelElement.getTerminals().iterator(); it.hasNext();) {
			Terminal childElement = (Terminal) it.next();
			int visualID = FritzingVisualIDRegistry.getNodeVisualID(view,
					childElement);
			if (visualID == Terminal2EditPart.VISUAL_ID) {
				result.add(new FritzingNodeDescriptor(childElement, visualID));
				continue;
			}
		}
		return result;
	}

	/**
	 * @generated
	 */
	public static List getSketch_1000SemanticChildren(View view) {
		if (!view.isSetElement()) {
			return Collections.EMPTY_LIST;
		}
		Sketch modelElement = (Sketch) view.getElement();
		List result = new LinkedList();
		for (Iterator it = modelElement.getParts().iterator(); it.hasNext();) {
			Part childElement = (Part) it.next();
			int visualID = FritzingVisualIDRegistry.getNodeVisualID(view,
					childElement);
			if (visualID == ArduinoEditPart.VISUAL_ID) {
				result.add(new FritzingNodeDescriptor(childElement, visualID));
				continue;
			}
			if (visualID == LEDEditPart.VISUAL_ID) {
				result.add(new FritzingNodeDescriptor(childElement, visualID));
				continue;
			}
			if (visualID == ResistorEditPart.VISUAL_ID) {
				result.add(new FritzingNodeDescriptor(childElement, visualID));
				continue;
			}
			if (visualID == ButtonEditPart.VISUAL_ID) {
				result.add(new FritzingNodeDescriptor(childElement, visualID));
				continue;
			}
		}
		for (Iterator it = modelElement.getTerminals().iterator(); it.hasNext();) {
			Terminal childElement = (Terminal) it.next();
			int visualID = FritzingVisualIDRegistry.getNodeVisualID(view,
					childElement);
			if (visualID == TerminalEditPart.VISUAL_ID) {
				result.add(new FritzingNodeDescriptor(childElement, visualID));
				continue;
			}
		}
		return result;
	}

	/**
	 * @generated
	 */
	public static List getContainedLinks(View view) {
		switch (FritzingVisualIDRegistry.getVisualID(view)) {
		case SketchEditPart.VISUAL_ID:
			return getSketch_1000ContainedLinks(view);
		case ArduinoEditPart.VISUAL_ID:
			return getArduino_2002ContainedLinks(view);
		case LEDEditPart.VISUAL_ID:
			return getLED_2004ContainedLinks(view);
		case ResistorEditPart.VISUAL_ID:
			return getResistor_2005ContainedLinks(view);
		case ButtonEditPart.VISUAL_ID:
			return getButton_2006ContainedLinks(view);
		case TerminalEditPart.VISUAL_ID:
			return getTerminal_2007ContainedLinks(view);
		case Terminal2EditPart.VISUAL_ID:
			return getTerminal_3001ContainedLinks(view);
		case WireEditPart.VISUAL_ID:
			return getWire_4001ContainedLinks(view);
		}
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getIncomingLinks(View view) {
		switch (FritzingVisualIDRegistry.getVisualID(view)) {
		case ArduinoEditPart.VISUAL_ID:
			return getArduino_2002IncomingLinks(view);
		case LEDEditPart.VISUAL_ID:
			return getLED_2004IncomingLinks(view);
		case ResistorEditPart.VISUAL_ID:
			return getResistor_2005IncomingLinks(view);
		case ButtonEditPart.VISUAL_ID:
			return getButton_2006IncomingLinks(view);
		case TerminalEditPart.VISUAL_ID:
			return getTerminal_2007IncomingLinks(view);
		case Terminal2EditPart.VISUAL_ID:
			return getTerminal_3001IncomingLinks(view);
		case WireEditPart.VISUAL_ID:
			return getWire_4001IncomingLinks(view);
		}
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getOutgoingLinks(View view) {
		switch (FritzingVisualIDRegistry.getVisualID(view)) {
		case ArduinoEditPart.VISUAL_ID:
			return getArduino_2002OutgoingLinks(view);
		case LEDEditPart.VISUAL_ID:
			return getLED_2004OutgoingLinks(view);
		case ResistorEditPart.VISUAL_ID:
			return getResistor_2005OutgoingLinks(view);
		case ButtonEditPart.VISUAL_ID:
			return getButton_2006OutgoingLinks(view);
		case TerminalEditPart.VISUAL_ID:
			return getTerminal_2007OutgoingLinks(view);
		case Terminal2EditPart.VISUAL_ID:
			return getTerminal_3001OutgoingLinks(view);
		case WireEditPart.VISUAL_ID:
			return getWire_4001OutgoingLinks(view);
		}
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getSketch_1000ContainedLinks(View view) {
		Sketch modelElement = (Sketch) view.getElement();
		List result = new LinkedList();
		result.addAll(getContainedTypeModelFacetLinks_Wire_4001(modelElement));
		return result;
	}

	/**
	 * @generated
	 */
	public static List getArduino_2002ContainedLinks(View view) {
		Arduino modelElement = (Arduino) view.getElement();
		List result = new LinkedList();
		result.addAll(getContainedTypeModelFacetLinks_Wire_4001(modelElement));
		return result;
	}

	/**
	 * @generated
	 */
	public static List getLED_2004ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getResistor_2005ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getButton_2006ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getTerminal_2007ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getTerminal_3001ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getWire_4001ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getArduino_2002IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getLED_2004IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getResistor_2005IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getButton_2006IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getTerminal_2007IncomingLinks(View view) {
		Terminal modelElement = (Terminal) view.getElement();
		Map crossReferences = EcoreUtil.CrossReferencer.find(view.eResource()
				.getResourceSet().getResources());
		List result = new LinkedList();
		result.addAll(getIncomingTypeModelFacetLinks_Wire_4001(modelElement,
				crossReferences));
		return result;
	}

	/**
	 * @generated
	 */
	public static List getTerminal_3001IncomingLinks(View view) {
		Terminal modelElement = (Terminal) view.getElement();
		Map crossReferences = EcoreUtil.CrossReferencer.find(view.eResource()
				.getResourceSet().getResources());
		List result = new LinkedList();
		result.addAll(getIncomingTypeModelFacetLinks_Wire_4001(modelElement,
				crossReferences));
		return result;
	}

	/**
	 * @generated
	 */
	public static List getWire_4001IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getArduino_2002OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getLED_2004OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getResistor_2005OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getButton_2006OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getTerminal_2007OutgoingLinks(View view) {
		Terminal modelElement = (Terminal) view.getElement();
		List result = new LinkedList();
		result.addAll(getOutgoingTypeModelFacetLinks_Wire_4001(modelElement));
		return result;
	}

	/**
	 * @generated
	 */
	public static List getTerminal_3001OutgoingLinks(View view) {
		Terminal modelElement = (Terminal) view.getElement();
		List result = new LinkedList();
		result.addAll(getOutgoingTypeModelFacetLinks_Wire_4001(modelElement));
		return result;
	}

	/**
	 * @generated
	 */
	public static List getWire_4001OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	private static Collection getContainedTypeModelFacetLinks_Wire_4001(
			Composite container) {
		Collection result = new LinkedList();
		for (Iterator links = container.getWires().iterator(); links.hasNext();) {
			Object linkObject = links.next();
			if (false == linkObject instanceof Wire) {
				continue;
			}
			Wire link = (Wire) linkObject;
			if (WireEditPart.VISUAL_ID != FritzingVisualIDRegistry
					.getLinkWithClassVisualID(link)) {
				continue;
			}
			Terminal dst = link.getTarget();
			Terminal src = link.getSource();
			result.add(new FritzingLinkDescriptor(src, dst, link,
					FritzingElementTypes.Wire_4001, WireEditPart.VISUAL_ID));
		}
		return result;
	}

	/**
	 * @generated
	 */
	private static Collection getIncomingTypeModelFacetLinks_Wire_4001(
			Terminal target, Map crossReferences) {
		Collection result = new LinkedList();
		Collection settings = (Collection) crossReferences.get(target);
		for (Iterator it = settings.iterator(); it.hasNext();) {
			EStructuralFeature.Setting setting = (EStructuralFeature.Setting) it
					.next();
			if (setting.getEStructuralFeature() != FritzingPackage.eINSTANCE
					.getWire_Target()
					|| false == setting.getEObject() instanceof Wire) {
				continue;
			}
			Wire link = (Wire) setting.getEObject();
			if (WireEditPart.VISUAL_ID != FritzingVisualIDRegistry
					.getLinkWithClassVisualID(link)) {
				continue;
			}
			Terminal src = link.getSource();
			result.add(new FritzingLinkDescriptor(src, target, link,
					FritzingElementTypes.Wire_4001, WireEditPart.VISUAL_ID));
		}
		return result;
	}

	/**
	 * @generated
	 */
	private static Collection getOutgoingTypeModelFacetLinks_Wire_4001(
			Terminal source) {
		Composite container = null;
		// Find container element for the link.
		// Climb up by containment hierarchy starting from the source
		// and return the first element that is instance of the container class.
		for (EObject element = source; element != null && container == null; element = element
				.eContainer()) {
			if (element instanceof Composite) {
				container = (Composite) element;
			}
		}
		if (container == null) {
			return Collections.EMPTY_LIST;
		}
		Collection result = new LinkedList();
		for (Iterator links = container.getWires().iterator(); links.hasNext();) {
			Object linkObject = links.next();
			if (false == linkObject instanceof Wire) {
				continue;
			}
			Wire link = (Wire) linkObject;
			if (WireEditPart.VISUAL_ID != FritzingVisualIDRegistry
					.getLinkWithClassVisualID(link)) {
				continue;
			}
			Terminal dst = link.getTarget();
			Terminal src = link.getSource();
			if (src != source) {
				continue;
			}
			result.add(new FritzingLinkDescriptor(src, dst, link,
					FritzingElementTypes.Wire_4001, WireEditPart.VISUAL_ID));
		}
		return result;
	}

}
