/**
 * (c) Fachhochschule Potsdam
 *
 * $Id$
 */
package org.fritzing.fritzing;

import org.eclipse.emf.ecore.EObject;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>IElement</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.fritzing.fritzing.IElement#getName <em>Name</em>}</li>
 *   <li>{@link org.fritzing.fritzing.IElement#getId <em>Id</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.fritzing.fritzing.FritzingPackage#getIElement()
 * @model interface="true" abstract="true"
 * @generated
 */
public interface IElement extends EObject {
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
	 * @see org.fritzing.fritzing.FritzingPackage#getIElement_Name()
	 * @model unique="false" dataType="org.eclipse.emf.ecore.xml.type.String"
	 *        extendedMetaData="name='name' kind='attribute'"
	 * @generated
	 */
	String getName();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.IElement#getName <em>Name</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Name</em>' attribute.
	 * @see #getName()
	 * @generated
	 */
	void setName(String value);

	/**
	 * Returns the value of the '<em><b>Id</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Id</em>' attribute isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Id</em>' attribute.
	 * @see #setId(String)
	 * @see org.fritzing.fritzing.FritzingPackage#getIElement_Id()
	 * @model dataType="org.eclipse.emf.ecore.xml.type.String"
	 *        extendedMetaData="name='id' kind='attribute'"
	 * @generated
	 */
	String getId();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.IElement#getId <em>Id</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Id</em>' attribute.
	 * @see #getId()
	 * @generated
	 */
	void setId(String value);

} // IElement
