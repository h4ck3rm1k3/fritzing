/**
 * (c) Fachhochschule Potsdam
 *
 * $Id$
 */
package org.fritzing.fritzing;


/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Leg</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.fritzing.fritzing.Leg#getParent <em>Parent</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Leg#getTarget <em>Target</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Leg#getSource <em>Source</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.fritzing.fritzing.FritzingPackage#getLeg()
 * @model extendedMetaData="name='Leg' kind='empty'"
 * @generated
 */
public interface Leg extends Connection, ILegConnection {
	/**
	 * Returns the value of the '<em><b>Parent</b></em>' container reference.
	 * It is bidirectional and its opposite is '{@link org.fritzing.fritzing.Terminal#getLeg <em>Leg</em>}'.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Parent</em>' container reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Parent</em>' container reference.
	 * @see #setParent(Terminal)
	 * @see org.fritzing.fritzing.FritzingPackage#getLeg_Parent()
	 * @see org.fritzing.fritzing.Terminal#getLeg
	 * @model opposite="leg" keys="id"
	 * @generated
	 */
	Terminal getParent();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Leg#getParent <em>Parent</em>}' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Parent</em>' container reference.
	 * @see #getParent()
	 * @generated
	 */
	void setParent(Terminal value);

	/**
	 * Returns the value of the '<em><b>Target</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Target</em>' reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Target</em>' reference.
	 * @see #setTarget(ILegConnection)
	 * @see org.fritzing.fritzing.FritzingPackage#getLeg_Target()
	 * @model extendedMetaData="kind='attribute' name='target'"
	 * @generated
	 */
	ILegConnection getTarget();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Leg#getTarget <em>Target</em>}' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Target</em>' reference.
	 * @see #getTarget()
	 * @generated
	 */
	void setTarget(ILegConnection value);

	/**
	 * Returns the value of the '<em><b>Source</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Source</em>' reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Source</em>' reference.
	 * @see #setSource(ILegConnection)
	 * @see org.fritzing.fritzing.FritzingPackage#getLeg_Source()
	 * @model required="true"
	 *        extendedMetaData="kind='attribute' name='source'"
	 * @generated
	 */
	ILegConnection getSource();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Leg#getSource <em>Source</em>}' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Source</em>' reference.
	 * @see #getSource()
	 * @generated
	 */
	void setSource(ILegConnection value);

} // Leg
