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
import org.fritzing.fritzing.FsrSensor;
import org.fritzing.fritzing.GenericPart;
import org.fritzing.fritzing.LightSensor;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.Potentiometer;
import org.fritzing.fritzing.PowerTransistor;
import org.fritzing.fritzing.Resistor;
import org.fritzing.fritzing.Sketch;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.Transistor;
import org.fritzing.fritzing.Wire;
import org.fritzing.fritzing.diagram.edit.parts.ArduinoEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ButtonEditPart;
import org.fritzing.fritzing.diagram.edit.parts.FsrSensorEditPart;
import org.fritzing.fritzing.diagram.edit.parts.GenericPartEditPart;
import org.fritzing.fritzing.diagram.edit.parts.LEDEditPart;
import org.fritzing.fritzing.diagram.edit.parts.LightSensorEditPart;
import org.fritzing.fritzing.diagram.edit.parts.PotentiometerEditPart;
import org.fritzing.fritzing.diagram.edit.parts.PowerTransistorEditPart;
import org.fritzing.fritzing.diagram.edit.parts.ResistorEditPart;
import org.fritzing.fritzing.diagram.edit.parts.SketchEditPart;
import org.fritzing.fritzing.diagram.edit.parts.Terminal2EditPart;
import org.fritzing.fritzing.diagram.edit.parts.TerminalEditPart;
import org.fritzing.fritzing.diagram.edit.parts.TransistorEditPart;
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
			return getArduino_2001SemanticChildren(view);
		case LEDEditPart.VISUAL_ID:
			return getLED_2002SemanticChildren(view);
		case ResistorEditPart.VISUAL_ID:
			return getResistor_2003SemanticChildren(view);
		case ButtonEditPart.VISUAL_ID:
			return getButton_2004SemanticChildren(view);
		case PotentiometerEditPart.VISUAL_ID:
			return getPotentiometer_2005SemanticChildren(view);
		case FsrSensorEditPart.VISUAL_ID:
			return getFsrSensor_2006SemanticChildren(view);
		case LightSensorEditPart.VISUAL_ID:
			return getLightSensor_2007SemanticChildren(view);
		case TransistorEditPart.VISUAL_ID:
			return getTransistor_2009SemanticChildren(view);
		case PowerTransistorEditPart.VISUAL_ID:
			return getPowerTransistor_2010SemanticChildren(view);
		case GenericPartEditPart.VISUAL_ID:
			return getGenericPart_2011SemanticChildren(view);
		case SketchEditPart.VISUAL_ID:
			return getSketch_1000SemanticChildren(view);
		}
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getArduino_2001SemanticChildren(View view) {
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
	public static List getLED_2002SemanticChildren(View view) {
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
	public static List getResistor_2003SemanticChildren(View view) {
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
	public static List getButton_2004SemanticChildren(View view) {
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
	public static List getPotentiometer_2005SemanticChildren(View view) {
		if (!view.isSetElement()) {
			return Collections.EMPTY_LIST;
		}
		Potentiometer modelElement = (Potentiometer) view.getElement();
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
	public static List getFsrSensor_2006SemanticChildren(View view) {
		if (!view.isSetElement()) {
			return Collections.EMPTY_LIST;
		}
		FsrSensor modelElement = (FsrSensor) view.getElement();
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
	public static List getLightSensor_2007SemanticChildren(View view) {
		if (!view.isSetElement()) {
			return Collections.EMPTY_LIST;
		}
		LightSensor modelElement = (LightSensor) view.getElement();
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
	public static List getTransistor_2009SemanticChildren(View view) {
		if (!view.isSetElement()) {
			return Collections.EMPTY_LIST;
		}
		Transistor modelElement = (Transistor) view.getElement();
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
	public static List getPowerTransistor_2010SemanticChildren(View view) {
		if (!view.isSetElement()) {
			return Collections.EMPTY_LIST;
		}
		PowerTransistor modelElement = (PowerTransistor) view.getElement();
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
	public static List getGenericPart_2011SemanticChildren(View view) {
		if (!view.isSetElement()) {
			return Collections.EMPTY_LIST;
		}
		GenericPart modelElement = (GenericPart) view.getElement();
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
			if (visualID == PotentiometerEditPart.VISUAL_ID) {
				result.add(new FritzingNodeDescriptor(childElement, visualID));
				continue;
			}
			if (visualID == FsrSensorEditPart.VISUAL_ID) {
				result.add(new FritzingNodeDescriptor(childElement, visualID));
				continue;
			}
			if (visualID == LightSensorEditPart.VISUAL_ID) {
				result.add(new FritzingNodeDescriptor(childElement, visualID));
				continue;
			}
			if (visualID == TransistorEditPart.VISUAL_ID) {
				result.add(new FritzingNodeDescriptor(childElement, visualID));
				continue;
			}
			if (visualID == PowerTransistorEditPart.VISUAL_ID) {
				result.add(new FritzingNodeDescriptor(childElement, visualID));
				continue;
			}
			if (visualID == GenericPartEditPart.VISUAL_ID) {
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
			return getArduino_2001ContainedLinks(view);
		case LEDEditPart.VISUAL_ID:
			return getLED_2002ContainedLinks(view);
		case ResistorEditPart.VISUAL_ID:
			return getResistor_2003ContainedLinks(view);
		case ButtonEditPart.VISUAL_ID:
			return getButton_2004ContainedLinks(view);
		case PotentiometerEditPart.VISUAL_ID:
			return getPotentiometer_2005ContainedLinks(view);
		case FsrSensorEditPart.VISUAL_ID:
			return getFsrSensor_2006ContainedLinks(view);
		case LightSensorEditPart.VISUAL_ID:
			return getLightSensor_2007ContainedLinks(view);
		case TerminalEditPart.VISUAL_ID:
			return getTerminal_2008ContainedLinks(view);
		case TransistorEditPart.VISUAL_ID:
			return getTransistor_2009ContainedLinks(view);
		case PowerTransistorEditPart.VISUAL_ID:
			return getPowerTransistor_2010ContainedLinks(view);
		case GenericPartEditPart.VISUAL_ID:
			return getGenericPart_2011ContainedLinks(view);
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
			return getArduino_2001IncomingLinks(view);
		case LEDEditPart.VISUAL_ID:
			return getLED_2002IncomingLinks(view);
		case ResistorEditPart.VISUAL_ID:
			return getResistor_2003IncomingLinks(view);
		case ButtonEditPart.VISUAL_ID:
			return getButton_2004IncomingLinks(view);
		case PotentiometerEditPart.VISUAL_ID:
			return getPotentiometer_2005IncomingLinks(view);
		case FsrSensorEditPart.VISUAL_ID:
			return getFsrSensor_2006IncomingLinks(view);
		case LightSensorEditPart.VISUAL_ID:
			return getLightSensor_2007IncomingLinks(view);
		case TerminalEditPart.VISUAL_ID:
			return getTerminal_2008IncomingLinks(view);
		case TransistorEditPart.VISUAL_ID:
			return getTransistor_2009IncomingLinks(view);
		case PowerTransistorEditPart.VISUAL_ID:
			return getPowerTransistor_2010IncomingLinks(view);
		case GenericPartEditPart.VISUAL_ID:
			return getGenericPart_2011IncomingLinks(view);
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
			return getArduino_2001OutgoingLinks(view);
		case LEDEditPart.VISUAL_ID:
			return getLED_2002OutgoingLinks(view);
		case ResistorEditPart.VISUAL_ID:
			return getResistor_2003OutgoingLinks(view);
		case ButtonEditPart.VISUAL_ID:
			return getButton_2004OutgoingLinks(view);
		case PotentiometerEditPart.VISUAL_ID:
			return getPotentiometer_2005OutgoingLinks(view);
		case FsrSensorEditPart.VISUAL_ID:
			return getFsrSensor_2006OutgoingLinks(view);
		case LightSensorEditPart.VISUAL_ID:
			return getLightSensor_2007OutgoingLinks(view);
		case TerminalEditPart.VISUAL_ID:
			return getTerminal_2008OutgoingLinks(view);
		case TransistorEditPart.VISUAL_ID:
			return getTransistor_2009OutgoingLinks(view);
		case PowerTransistorEditPart.VISUAL_ID:
			return getPowerTransistor_2010OutgoingLinks(view);
		case GenericPartEditPart.VISUAL_ID:
			return getGenericPart_2011OutgoingLinks(view);
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
	public static List getArduino_2001ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getLED_2002ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getResistor_2003ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getButton_2004ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getPotentiometer_2005ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getFsrSensor_2006ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getLightSensor_2007ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getTerminal_2008ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getTransistor_2009ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getPowerTransistor_2010ContainedLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getGenericPart_2011ContainedLinks(View view) {
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
	public static List getArduino_2001IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getLED_2002IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getResistor_2003IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getButton_2004IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getPotentiometer_2005IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getFsrSensor_2006IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getLightSensor_2007IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getTerminal_2008IncomingLinks(View view) {
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
	public static List getTransistor_2009IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getPowerTransistor_2010IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getGenericPart_2011IncomingLinks(View view) {
		return Collections.EMPTY_LIST;
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
	public static List getArduino_2001OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getLED_2002OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getResistor_2003OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getButton_2004OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getPotentiometer_2005OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getFsrSensor_2006OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getLightSensor_2007OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getTerminal_2008OutgoingLinks(View view) {
		Terminal modelElement = (Terminal) view.getElement();
		List result = new LinkedList();
		result.addAll(getOutgoingTypeModelFacetLinks_Wire_4001(modelElement));
		return result;
	}

	/**
	 * @generated
	 */
	public static List getTransistor_2009OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getPowerTransistor_2010OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
	}

	/**
	 * @generated
	 */
	public static List getGenericPart_2011OutgoingLinks(View view) {
		return Collections.EMPTY_LIST;
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
