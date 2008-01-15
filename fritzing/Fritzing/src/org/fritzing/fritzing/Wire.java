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
 *   <li>{@link org.fritzing.fritzing.Wire#getParent <em>Parent</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Wire#getTarget <em>Target</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Wire#getSource <em>Source</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.fritzing.fritzing.FritzingPackage#getWire()
 * @model extendedMetaData="name='Wire' kind='empty'"
 * @generated
 */
public interface Wire extends Connection, ILegConnection {
	/**
	 * Returns the value of the '<em><b>Source</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Source</em>' reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Source</em>' reference.
	 * @see #setSource(IWireConnection)
	 * @see org.fritzing.fritzing.FritzingPackage#getWire_Source()
	 * @model keys="id" required="true"
	 *        extendedMetaData="kind='attribute' name='source'"
	 * @generated
	 */
	IWireConnection getSource();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Wire#getSource <em>Source</em>}' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Source</em>' reference.
	 * @see #getSource()
	 * @generated
	 */
	void setSource(IWireConnection value);

	/**
	 * Returns the value of the '<em><b>Target</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Target</em>' reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Target</em>' reference.
	 * @see #setTarget(IWireConnection)
	 * @see org.fritzing.fritzing.FritzingPackage#getWire_Target()
	 * @model keys="id" required="true"
	 *        extendedMetaData="kind='attribute' name='target'"
	 * @generated
	 */
	IWireConnection getTarget();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Wire#getTarget <em>Target</em>}' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Target</em>' reference.
	 * @see #getTarget()
	 * @generated
	 */
	void setTarget(IWireConnection value);

	/**
	 * Returns the value of the '<em><b>Parent</b></em>' container reference.
	 * It is bidirectional and its opposite is '{@link org.fritzing.fritzing.Composite#getWires <em>Wires</em>}'.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Parent</em>' container reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Parent</em>' container reference.
	 * @see #setParent(Composite)
	 * @see org.fritzing.fritzing.FritzingPackage#getWire_Parent()
	 * @see org.fritzing.fritzing.Composite#getWires
	 * @model opposite="wires"
	 * @generated
	 */
	Composite getParent();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Wire#getParent <em>Parent</em>}' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Parent</em>' container reference.
	 * @see #getParent()
	 * @generated
	 */
	void setParent(Composite value);

} // Wire
