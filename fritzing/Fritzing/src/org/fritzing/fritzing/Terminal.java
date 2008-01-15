/**
 * (c) Fachhochschule Potsdam
 *
 * $Id$
 */
package org.fritzing.fritzing;


/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Terminal</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.fritzing.fritzing.Terminal#getParent <em>Parent</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Terminal#getLeg <em>Leg</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.fritzing.fritzing.FritzingPackage#getTerminal()
 * @model extendedMetaData="name='Terminal' kind='empty'"
 * @generated
 */
public interface Terminal extends Element, ITrackConnection, ILegConnection, IWireConnection {
	/**
	 * Returns the value of the '<em><b>Parent</b></em>' container reference.
	 * It is bidirectional and its opposite is '{@link org.fritzing.fritzing.Part#getTerminals <em>Terminals</em>}'.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Parent</em>' container reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Parent</em>' container reference.
	 * @see #setParent(Part)
	 * @see org.fritzing.fritzing.FritzingPackage#getTerminal_Parent()
	 * @see org.fritzing.fritzing.Part#getTerminals
	 * @model opposite="terminals"
	 * @generated
	 */
	Part getParent();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Terminal#getParent <em>Parent</em>}' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Parent</em>' container reference.
	 * @see #getParent()
	 * @generated
	 */
	void setParent(Part value);

	/**
	 * Returns the value of the '<em><b>Leg</b></em>' containment reference.
	 * It is bidirectional and its opposite is '{@link org.fritzing.fritzing.Leg#getParent <em>Parent</em>}'.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Leg</em>' containment reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Leg</em>' containment reference.
	 * @see #setLeg(Leg)
	 * @see org.fritzing.fritzing.FritzingPackage#getTerminal_Leg()
	 * @see org.fritzing.fritzing.Leg#getParent
	 * @model opposite="parent" containment="true" keys="id"
	 * @generated
	 */
	Leg getLeg();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Terminal#getLeg <em>Leg</em>}' containment reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Leg</em>' containment reference.
	 * @see #getLeg()
	 * @generated
	 */
	void setLeg(Leg value);

} // Terminal
