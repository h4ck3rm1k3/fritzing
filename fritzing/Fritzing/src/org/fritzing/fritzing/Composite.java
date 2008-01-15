/**
 * (c) Fachhochschule Potsdam
 *
 * $Id$
 */
package org.fritzing.fritzing;

import org.eclipse.emf.common.util.EList;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Composite</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.fritzing.fritzing.Composite#getParts <em>Parts</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Composite#getWires <em>Wires</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.fritzing.fritzing.FritzingPackage#getComposite()
 * @model abstract="true"
 *        extendedMetaData="name='Composite' kind='elementOnly'"
 * @generated
 */
public interface Composite extends Part {
	/**
	 * Returns the value of the '<em><b>Parts</b></em>' containment reference list.
	 * The list contents are of type {@link org.fritzing.fritzing.Part}.
	 * It is bidirectional and its opposite is '{@link org.fritzing.fritzing.Part#getParent <em>Parent</em>}'.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Parts</em>' containment reference list isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Parts</em>' containment reference list.
	 * @see org.fritzing.fritzing.FritzingPackage#getComposite_Parts()
	 * @see org.fritzing.fritzing.Part#getParent
	 * @model opposite="parent" containment="true" keys="id"
	 *        extendedMetaData="kind='element' name='part'"
	 * @generated
	 */
	EList<Part> getParts();

	/**
	 * Returns the value of the '<em><b>Wires</b></em>' containment reference list.
	 * The list contents are of type {@link org.fritzing.fritzing.Wire}.
	 * It is bidirectional and its opposite is '{@link org.fritzing.fritzing.Wire#getParent <em>Parent</em>}'.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Wires</em>' containment reference list isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Wires</em>' containment reference list.
	 * @see org.fritzing.fritzing.FritzingPackage#getComposite_Wires()
	 * @see org.fritzing.fritzing.Wire#getParent
	 * @model opposite="parent" containment="true" keys="id"
	 *        extendedMetaData="kind='element' name='wire'"
	 * @generated
	 */
	EList<Wire> getWires();

} // Composite
