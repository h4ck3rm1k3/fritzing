/**
 * (c) Fachhochschule Potsdam
 *
 * $Id$
 */
package org.fritzing.fritzing;

import org.eclipse.emf.ecore.EObject;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>IConnection Element</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.fritzing.fritzing.IConnectionElement#getId <em>Id</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.fritzing.fritzing.FritzingPackage#getIConnectionElement()
 * @model interface="true" abstract="true"
 * @generated
 */
public interface IConnectionElement extends EObject {
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
	 * @see org.fritzing.fritzing.FritzingPackage#getIConnectionElement_Id()
	 * @model unique="false" id="true" dataType="org.eclipse.emf.ecore.xml.type.String"
	 *        extendedMetaData="name='id' kind='attribute'"
	 * @generated
	 */
	String getId();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.IConnectionElement#getId <em>Id</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Id</em>' attribute.
	 * @see #getId()
	 * @generated
	 */
	void setId(String value);

} // IConnectionElement
