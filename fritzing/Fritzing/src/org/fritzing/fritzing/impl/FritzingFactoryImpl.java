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
			case FritzingPackage.WIRE: return createWire();
			case FritzingPackage.RESISTOR: return createResistor();
			case FritzingPackage.LED: return createLED();
			case FritzingPackage.BUTTON: return createButton();
			case FritzingPackage.SKETCH: return createSketch();
			case FritzingPackage.BREADBOARD: return createBreadboard();
			case FritzingPackage.MODULE: return createModule();
			case FritzingPackage.ARDUINO: return createArduino();
			case FritzingPackage.POTENTIOMETER: return createPotentiometer();
			case FritzingPackage.FSR_SENSOR: return createFsrSensor();
			case FritzingPackage.LIGHT_SENSOR: return createLightSensor();
			case FritzingPackage.TRANSISTOR: return createTransistor();
			case FritzingPackage.POWER_TRANSISTOR: return createPowerTransistor();
			case FritzingPackage.GENERIC_PART: return createGenericPart();
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
	public Button createButton() {
		ButtonImpl button = new ButtonImpl();
		return button;
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
	public Arduino createArduino() {
		ArduinoImpl arduino = new ArduinoImpl();
		return arduino;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public Potentiometer createPotentiometer() {
		PotentiometerImpl potentiometer = new PotentiometerImpl();
		return potentiometer;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public FsrSensor createFsrSensor() {
		FsrSensorImpl fsrSensor = new FsrSensorImpl();
		return fsrSensor;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public LightSensor createLightSensor() {
		LightSensorImpl lightSensor = new LightSensorImpl();
		return lightSensor;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public Transistor createTransistor() {
		TransistorImpl transistor = new TransistorImpl();
		return transistor;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public PowerTransistor createPowerTransistor() {
		PowerTransistorImpl powerTransistor = new PowerTransistorImpl();
		return powerTransistor;
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
