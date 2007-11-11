/**
 * (c) Fachhochschule Potsdam
 *
 * $Id$
 */
package org.fritzing.fritzing;

import org.eclipse.emf.ecore.EFactory;

/**
 * <!-- begin-user-doc -->
 * The <b>Factory</b> for the model.
 * It provides a create method for each non-abstract class of the model.
 * <!-- end-user-doc -->
 * @see org.fritzing.fritzing.FritzingPackage
 * @generated
 */
public interface FritzingFactory extends EFactory {
	/**
	 * The singleton instance of the factory.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	FritzingFactory eINSTANCE = org.fritzing.fritzing.impl.FritzingFactoryImpl.init();

	/**
	 * Returns a new object of class '<em>Document Root</em>'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return a new object of class '<em>Document Root</em>'.
	 * @generated
	 */
	DocumentRoot createDocumentRoot();

	/**
	 * Returns a new object of class '<em>Terminal</em>'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return a new object of class '<em>Terminal</em>'.
	 * @generated
	 */
	Terminal createTerminal();

	/**
	 * Returns a new object of class '<em>Wire</em>'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return a new object of class '<em>Wire</em>'.
	 * @generated
	 */
	Wire createWire();

	/**
	 * Returns a new object of class '<em>Resistor</em>'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return a new object of class '<em>Resistor</em>'.
	 * @generated
	 */
	Resistor createResistor();

	/**
	 * Returns a new object of class '<em>LED</em>'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return a new object of class '<em>LED</em>'.
	 * @generated
	 */
	LED createLED();

	/**
	 * Returns a new object of class '<em>Button</em>'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return a new object of class '<em>Button</em>'.
	 * @generated
	 */
	Button createButton();

	/**
	 * Returns a new object of class '<em>Sketch</em>'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return a new object of class '<em>Sketch</em>'.
	 * @generated
	 */
	Sketch createSketch();

	/**
	 * Returns a new object of class '<em>Breadboard</em>'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return a new object of class '<em>Breadboard</em>'.
	 * @generated
	 */
	Breadboard createBreadboard();

	/**
	 * Returns a new object of class '<em>Module</em>'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return a new object of class '<em>Module</em>'.
	 * @generated
	 */
	Module createModule();

	/**
	 * Returns a new object of class '<em>Arduino</em>'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return a new object of class '<em>Arduino</em>'.
	 * @generated
	 */
	Arduino createArduino();

	/**
	 * Returns a new object of class '<em>Potentiometer</em>'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return a new object of class '<em>Potentiometer</em>'.
	 * @generated
	 */
	Potentiometer createPotentiometer();

	/**
	 * Returns a new object of class '<em>Fsr Sensor</em>'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return a new object of class '<em>Fsr Sensor</em>'.
	 * @generated
	 */
	FsrSensor createFsrSensor();

	/**
	 * Returns a new object of class '<em>Light Sensor</em>'.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return a new object of class '<em>Light Sensor</em>'.
	 * @generated
	 */
	LightSensor createLightSensor();

	/**
	 * Returns the package supported by this factory.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the package supported by this factory.
	 * @generated
	 */
	FritzingPackage getFritzingPackage();

} //FritzingFactory
