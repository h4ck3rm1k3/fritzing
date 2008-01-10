/**
 * (c) Fachhochschule Potsdam
 *
 * $Id$
 */
package org.fritzing.fritzing.impl;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.EPackage;

import org.eclipse.emf.ecore.impl.EFactoryImpl;

import org.eclipse.emf.ecore.plugin.EcorePlugin;

import org.fritzing.fritzing.*;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model <b>Factory</b>.
 * <!-- end-user-doc -->
 * @generated
 */
public class FritzingFactoryImpl extends EFactoryImpl implements FritzingFactory {
	/**
	 * Creates the default factory implementation.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public static FritzingFactory init() {
		try {
			FritzingFactory theFritzingFactory = (FritzingFactory)EPackage.Registry.INSTANCE.getEFactory("http://www.fritzing.org"); 
			if (theFritzingFactory != null) {
				return theFritzingFactory;
			}
		}
		catch (Exception exception) {
			EcorePlugin.INSTANCE.log(exception);
		}
		return new FritzingFactoryImpl();
	}

	/**
	 * Creates an instance of the factory.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public FritzingFactoryImpl() {
		super();
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	@Override
	public EObject create(EClass eClass) {
		switch (eClass.getClassifierID()) {
			case FritzingPackage.DOCUMENT_ROOT: return createDocumentRoot();
			case FritzingPackage.TERMINAL: return createTerminal();
			case FritzingPackage.GENERIC_PART: return createGenericPart();
			case FritzingPackage.RESISTOR: return createResistor();
			case FritzingPackage.LED: return createLED();
			case FritzingPackage.SKETCH: return createSketch();
			case FritzingPackage.MODULE: return createModule();
			case FritzingPackage.BREADBOARD: return createBreadboard();
			case FritzingPackage.WIRE: return createWire();
			case FritzingPackage.TRACK: return createTrack();
			case FritzingPackage.LEG: return createLeg();
			default:
				throw new IllegalArgumentException("The class '" + eClass.getName() + "' is not a valid classifier");
		}
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public DocumentRoot createDocumentRoot() {
		DocumentRootImpl documentRoot = new DocumentRootImpl();
		return documentRoot;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public Terminal createTerminal() {
		TerminalImpl terminal = new TerminalImpl();
		return terminal;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public Wire createWire() {
		WireImpl wire = new WireImpl();
		return wire;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public Track createTrack() {
		TrackImpl track = new TrackImpl();
		return track;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public Leg createLeg() {
		LegImpl leg = new LegImpl();
		return leg;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public Resistor createResistor() {
		ResistorImpl resistor = new ResistorImpl();
		return resistor;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public LED createLED() {
		LEDImpl led = new LEDImpl();
		return led;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public Sketch createSketch() {
		SketchImpl sketch = new SketchImpl();
		return sketch;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public Breadboard createBreadboard() {
		BreadboardImpl breadboard = new BreadboardImpl();
		return breadboard;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public Module createModule() {
		ModuleImpl module = new ModuleImpl();
		return module;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public GenericPart createGenericPart() {
		GenericPartImpl genericPart = new GenericPartImpl();
		return genericPart;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public FritzingPackage getFritzingPackage() {
		return (FritzingPackage)getEPackage();
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @deprecated
	 * @generated
	 */
	@Deprecated
	public static FritzingPackage getPackage() {
		return FritzingPackage.eINSTANCE;
	}

} //FritzingFactoryImpl
