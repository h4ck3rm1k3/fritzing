/**
 * (c) Fachhochschule Potsdam
 *
 * $Id$
 */
package org.fritzing.fritzing;

import org.eclipse.emf.common.util.EList;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Part</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.fritzing.fritzing.Part#getName <em>Name</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Part#getPartNumber <em>Part Number</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Part#getTerminals <em>Terminals</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Part#getParent <em>Parent</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.fritzing.fritzing.FritzingPackage#getPart()
 * @model abstract="true"
 *        extendedMetaData="name='Part' kind='elementOnly'"
 * @generated
 */
public interface Part extends Element {
	/**
	 * Returns the value of the '<em><b>Name</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Name</em>' attribute isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Name</em>' attribute.
	 * @see #setName(String)
	 * @see org.fritzing.fritzing.FritzingPackage#getPart_Name()
	 * @model id="true" dataType="org.eclipse.emf.ecore.xml.type.String"
	 *        extendedMetaData="name='name' kind='attribute'"
	 * @generated
	 */
	String getName();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Part#getName <em>Name</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Name</em>' attribute.
	 * @see #getName()
	 * @generated
	 */
	void setName(String value);

	/**
	 * Returns the value of the '<em><b>Part Number</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Part Number</em>' attribute isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Part Number</em>' attribute.
	 * @see #setPartNumber(String)
	 * @see org.fritzing.fritzing.FritzingPackage#getPart_PartNumber()
	 * @model unique="false" dataType="org.eclipse.emf.ecore.xml.type.String"
	 *        extendedMetaData="kind='attribute' name='partNumber'"
	 * @generated
	 */
	String getPartNumber();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Part#getPartNumber <em>Part Number</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Part Number</em>' attribute.
	 * @see #getPartNumber()
	 * @generated
	 */
	void setPartNumber(String value);

	/**
	 * Returns the value of the '<em><b>Terminals</b></em>' containment reference list.
	 * The list contents are of type {@link org.fritzing.fritzing.Terminal}.
	 * It is bidirectional and its opposite is '{@link org.fritzing.fritzing.Terminal#getParent <em>Parent</em>}'.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Terminals</em>' containment reference list isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Terminals</em>' containment reference list.
	 * @see org.fritzing.fritzing.FritzingPackage#getPart_Terminals()
	 * @see org.fritzing.fritzing.Terminal#getParent
	 * @model opposite="parent" containment="true" keys="name"
	 *        extendedMetaData="kind='element' name='terminal'"
	 * @generated
	 */
	EList<Terminal> getTerminals();

	/**
	 * Returns the value of the '<em><b>Parent</b></em>' container reference.
	 * It is bidirectional and its opposite is '{@link org.fritzing.fritzing.Composite#getParts <em>Parts</em>}'.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Parent</em>' container reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Parent</em>' container reference.
	 * @see #setParent(Composite)
	 * @see org.fritzing.fritzing.FritzingPackage#getPart_Parent()
	 * @see org.fritzing.fritzing.Composite#getParts
	 * @model opposite="parts" keys="name"
	 * @generated
	 */
	Composite getParent();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Part#getParent <em>Parent</em>}' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Parent</em>' container reference.
	 * @see #getParent()
	 * @generated
	 */
	void setParent(Composite value);

} // Part
