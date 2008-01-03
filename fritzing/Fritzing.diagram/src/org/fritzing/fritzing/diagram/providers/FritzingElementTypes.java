/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.providers;

import java.util.HashSet;
import java.util.IdentityHashMap;
import java.util.Map;
import java.util.Set;

import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EClassifier;
import org.eclipse.emf.ecore.ENamedElement;
import org.eclipse.emf.ecore.EStructuralFeature;
import org.eclipse.gmf.runtime.emf.type.core.ElementTypeRegistry;
import org.eclipse.gmf.runtime.emf.type.core.IElementType;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.resource.ImageRegistry;
import org.eclipse.swt.graphics.Image;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorPlugin;

/**
 * @generated
 */
public class FritzingElementTypes extends ElementInitializers {

	/**
	 * @generated
	 */
	private FritzingElementTypes() {
	}

	/**
	 * @generated
	 */
	private static Map elements;

	/**
	 * @generated
	 */
	private static ImageRegistry imageRegistry;

	/**
	 * @generated
	 */
	private static Set KNOWN_ELEMENT_TYPES;

	/**
	 * @generated
	 */
	public static final IElementType Sketch_1000 = getElementType("Fritzing.diagram.Sketch_1000"); //$NON-NLS-1$
	/**
	 * @generated
	 */
	public static final IElementType Arduino_2001 = getElementType("Fritzing.diagram.Arduino_2001"); //$NON-NLS-1$

	/**
	 * @generated
	 */
	public static final IElementType LED_2002 = getElementType("Fritzing.diagram.LED_2002"); //$NON-NLS-1$

	/**
	 * @generated
	 */
	public static final IElementType Resistor_2003 = getElementType("Fritzing.diagram.Resistor_2003"); //$NON-NLS-1$

	/**
	 * @generated
	 */
	public static final IElementType Button_2004 = getElementType("Fritzing.diagram.Button_2004"); //$NON-NLS-1$

	/**
	 * @generated
	 */
	public static final IElementType Potentiometer_2005 = getElementType("Fritzing.diagram.Potentiometer_2005"); //$NON-NLS-1$

	/**
	 * @generated
	 */
	public static final IElementType FsrSensor_2006 = getElementType("Fritzing.diagram.FsrSensor_2006"); //$NON-NLS-1$

	/**
	 * @generated
	 */
	public static final IElementType LightSensor_2007 = getElementType("Fritzing.diagram.LightSensor_2007"); //$NON-NLS-1$

	/**
	 * @generated
	 */
	public static final IElementType Terminal_2008 = getElementType("Fritzing.diagram.Terminal_2008"); //$NON-NLS-1$

	/**
	 * @generated
	 */
	public static final IElementType Transistor_2009 = getElementType("Fritzing.diagram.Transistor_2009"); //$NON-NLS-1$

	/**
	 * @generated
	 */
	public static final IElementType PowerTransistor_2010 = getElementType("Fritzing.diagram.PowerTransistor_2010"); //$NON-NLS-1$

	/**
	 * @generated
	 */
	public static final IElementType GenericPart_2011 = getElementType("Fritzing.diagram.GenericPart_2011"); //$NON-NLS-1$

	/**
	 * @generated
	 */
	public static final IElementType Terminal_3001 = getElementType("Fritzing.diagram.Terminal_3001"); //$NON-NLS-1$
	/**
	 * @generated
	 */
	public static final IElementType Wire_4001 = getElementType("Fritzing.diagram.Wire_4001"); //$NON-NLS-1$

	/**
	 * @generated NOT
	 */
	public static ImageRegistry getImageRegistry() {
		if (imageRegistry == null) {
			imageRegistry = new ImageRegistry();
		}
		return imageRegistry;
	}

	/**
	 * @generated NOT
	 */
	public static String getImageRegistryKey(ENamedElement element) {
		return element.getName();
	}

	/**
	 * @generated
	 */
	private static ImageDescriptor getProvidedImageDescriptor(
			ENamedElement element) {
		if (element instanceof EStructuralFeature) {
			EStructuralFeature feature = ((EStructuralFeature) element);
			EClass eContainingClass = feature.getEContainingClass();
			EClassifier eType = feature.getEType();
			if (eContainingClass != null && !eContainingClass.isAbstract()) {
				element = eContainingClass;
			} else if (eType instanceof EClass
					&& !((EClass) eType).isAbstract()) {
				element = eType;
			}
		}
		if (element instanceof EClass) {
			EClass eClass = (EClass) element;
			if (!eClass.isAbstract()) {
				return FritzingDiagramEditorPlugin.getInstance()
						.getItemImageDescriptor(
								eClass.getEPackage().getEFactoryInstance()
										.create(eClass));
			}
		}
		// TODO : support structural features
		return null;
	}

	/**
	 * @generated
	 */
	public static ImageDescriptor getImageDescriptor(ENamedElement element) {
		String key = getImageRegistryKey(element);
		ImageDescriptor imageDescriptor = getImageRegistry().getDescriptor(key);
		if (imageDescriptor == null) {
			imageDescriptor = getProvidedImageDescriptor(element);
			if (imageDescriptor == null) {
				imageDescriptor = ImageDescriptor.getMissingImageDescriptor();
			}
			getImageRegistry().put(key, imageDescriptor);
		}
		return imageDescriptor;
	}

	/**
	 * @generated
	 */
	public static Image getImage(ENamedElement element) {
		String key = getImageRegistryKey(element);
		Image image = getImageRegistry().get(key);
		if (image == null) {
			ImageDescriptor imageDescriptor = getProvidedImageDescriptor(element);
			if (imageDescriptor == null) {
				imageDescriptor = ImageDescriptor.getMissingImageDescriptor();
			}
			getImageRegistry().put(key, imageDescriptor);
			image = getImageRegistry().get(key);
		}
		return image;
	}

	/**
	 * @generated
	 */
	public static ImageDescriptor getImageDescriptor(IAdaptable hint) {
		ENamedElement element = getElement(hint);
		if (element == null) {
			return null;
		}
		return getImageDescriptor(element);
	}

	/**
	 * @generated
	 */
	public static Image getImage(IAdaptable hint) {
		ENamedElement element = getElement(hint);
		if (element == null) {
			return null;
		}
		return getImage(element);
	}

	/**
	 * Returns 'type' of the ecore object associated with the hint.
	 * 
	 * @generated
	 */
	public static ENamedElement getElement(IAdaptable hint) {
		Object type = hint.getAdapter(IElementType.class);
		if (elements == null) {
			elements = new IdentityHashMap();

			elements.put(Sketch_1000, FritzingPackage.eINSTANCE.getSketch());

			elements.put(Arduino_2001, FritzingPackage.eINSTANCE.getArduino());

			elements.put(LED_2002, FritzingPackage.eINSTANCE.getLED());

			elements
					.put(Resistor_2003, FritzingPackage.eINSTANCE.getResistor());

			elements.put(Button_2004, FritzingPackage.eINSTANCE.getButton());

			elements.put(Potentiometer_2005, FritzingPackage.eINSTANCE
					.getPotentiometer());

			elements.put(FsrSensor_2006, FritzingPackage.eINSTANCE
					.getFsrSensor());

			elements.put(LightSensor_2007, FritzingPackage.eINSTANCE
					.getLightSensor());

			elements
					.put(Terminal_2008, FritzingPackage.eINSTANCE.getTerminal());

			elements.put(Transistor_2009, FritzingPackage.eINSTANCE
					.getTransistor());

			elements.put(PowerTransistor_2010, FritzingPackage.eINSTANCE
					.getPowerTransistor());

			elements.put(GenericPart_2011, FritzingPackage.eINSTANCE
					.getGenericPart());

			elements
					.put(Terminal_3001, FritzingPackage.eINSTANCE.getTerminal());

			elements.put(Wire_4001, FritzingPackage.eINSTANCE.getWire());
		}
		return (ENamedElement) elements.get(type);
	}

	/**
	 * @generated
	 */
	private static IElementType getElementType(String id) {
		return ElementTypeRegistry.getInstance().getType(id);
	}

	/**
	 * @generated
	 */
	public static boolean isKnownElementType(IElementType elementType) {
		if (KNOWN_ELEMENT_TYPES == null) {
			KNOWN_ELEMENT_TYPES = new HashSet();
			KNOWN_ELEMENT_TYPES.add(Sketch_1000);
			KNOWN_ELEMENT_TYPES.add(Arduino_2001);
			KNOWN_ELEMENT_TYPES.add(LED_2002);
			KNOWN_ELEMENT_TYPES.add(Resistor_2003);
			KNOWN_ELEMENT_TYPES.add(Button_2004);
			KNOWN_ELEMENT_TYPES.add(Potentiometer_2005);
			KNOWN_ELEMENT_TYPES.add(FsrSensor_2006);
			KNOWN_ELEMENT_TYPES.add(LightSensor_2007);
			KNOWN_ELEMENT_TYPES.add(Terminal_2008);
			KNOWN_ELEMENT_TYPES.add(Transistor_2009);
			KNOWN_ELEMENT_TYPES.add(PowerTransistor_2010);
			KNOWN_ELEMENT_TYPES.add(GenericPart_2011);
			KNOWN_ELEMENT_TYPES.add(Terminal_3001);
			KNOWN_ELEMENT_TYPES.add(Wire_4001);
		}
		return KNOWN_ELEMENT_TYPES.contains(elementType);
	}

}
