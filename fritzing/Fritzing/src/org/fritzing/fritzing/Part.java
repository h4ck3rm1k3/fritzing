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
 *   <li>{@link org.fritzing.fritzing.Part#getSpecies <em>Species</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Part#getGenus <em>Genus</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Part#getVersion <em>Version</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Part#getDescription <em>Description</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Part#getFootprint <em>Footprint</em>}</li>
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
	 * Returns the value of the '<em><b>Species</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Species</em>' attribute isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Species</em>' attribute.
	 * @see #setSpecies(String)
	 * @see org.fritzing.fritzing.FritzingPackage#getPart_Species()
	 * @model dataType="org.eclipse.emf.ecore.xml.type.String"
	 *        extendedMetaData="name='species' kind='attribute'"
	 * @generated
	 */
	String getSpecies();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Part#getSpecies <em>Species</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Species</em>' attribute.
	 * @see #getSpecies()
	 * @generated
	 */
	void setSpecies(String value);

	/**
	 * Returns the value of the '<em><b>Genus</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Genus</em>' attribute isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Genus</em>' attribute.
	 * @see #setGenus(String)
	 * @see org.fritzing.fritzing.FritzingPackage#getPart_Genus()
	 * @model dataType="org.eclipse.emf.ecore.xml.type.String"
	 *        extendedMetaData="name='genus' kind='attribute'"
	 * @generated
	 */
	String getGenus();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Part#getGenus <em>Genus</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Genus</em>' attribute.
	 * @see #getGenus()
	 * @generated
	 */
	void setGenus(String value);

	/**
	 * Returns the value of the '<em><b>Version</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Version</em>' attribute isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Version</em>' attribute.
	 * @see #setVersion(String)
	 * @see org.fritzing.fritzing.FritzingPackage#getPart_Version()
	 * @model dataType="org.eclipse.emf.ecore.xml.type.String"
	 *        extendedMetaData="name='version' kind='attribute'"
	 * @generated
	 */
	String getVersion();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Part#getVersion <em>Version</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Version</em>' attribute.
	 * @see #getVersion()
	 * @generated
	 */
	void setVersion(String value);

	/**
	 * Returns the value of the '<em><b>Description</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Description</em>' attribute isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Description</em>' attribute.
	 * @see #setDescription(String)
	 * @see org.fritzing.fritzing.FritzingPackage#getPart_Description()
	 * @model dataType="org.eclipse.emf.ecore.xml.type.String"
	 *        extendedMetaData="name='description' kind='attribute'"
	 * @generated
	 */
	String getDescription();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Part#getDescription <em>Description</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Description</em>' attribute.
	 * @see #getDescription()
	 * @generated
	 */
	void setDescription(String value);

	/**
	 * Returns the value of the '<em><b>Footprint</b></em>' attribute.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Footprint</em>' attribute isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Footprint</em>' attribute.
	 * @see #setFootprint(String)
	 * @see org.fritzing.fritzing.FritzingPackage#getPart_Footprint()
	 * @model dataType="org.eclipse.emf.ecore.xml.type.String"
	 *        extendedMetaData="name='footprint' kind='attribute'"
	 * @generated
	 */
	String getFootprint();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Part#getFootprint <em>Footprint</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Footprint</em>' attribute.
	 * @see #getFootprint()
	 * @generated
	 */
	void setFootprint(String value);

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
