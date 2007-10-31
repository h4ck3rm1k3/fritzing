/**
 * (c) Fachhochschule Potsdam
 *
 * $Id$
 */
package org.fritzing.fritzing;


/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Wire</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.fritzing.fritzing.Wire#getSource <em>Source</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Wire#getTarget <em>Target</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.fritzing.fritzing.FritzingPackage#getWire()
 * @model extendedMetaData="name='Wire' kind='empty'"
 * @generated
 */
public interface Wire extends Element {
	/**
	 * Returns the value of the '<em><b>Source</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Source</em>' reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Source</em>' reference.
	 * @see #setSource(Terminal)
	 * @see org.fritzing.fritzing.FritzingPackage#getWire_Source()
	 * @model required="true"
	 *        extendedMetaData="kind='attribute' name='source'"
	 * @generated
	 */
	Terminal getSource();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Wire#getSource <em>Source</em>}' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Source</em>' reference.
	 * @see #getSource()
	 * @generated
	 */
	void setSource(Terminal value);

	/**
	 * Returns the value of the '<em><b>Target</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Target</em>' reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Target</em>' reference.
	 * @see #setTarget(Terminal)
	 * @see org.fritzing.fritzing.FritzingPackage#getWire_Target()
	 * @model required="true"
	 *        extendedMetaData="kind='attribute' name='target'"
	 * @generated
	 */
	Terminal getTarget();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Wire#getTarget <em>Target</em>}' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Target</em>' reference.
	 * @see #getTarget()
	 * @generated
	 */
	void setTarget(Terminal value);

} // Wire
