/**
 * (c) Fachhochschule Potsdam
 *
 * $Id$
 */
package org.fritzing.fritzing;


/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Track</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.fritzing.fritzing.Track#getParent <em>Parent</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Track#getTarget <em>Target</em>}</li>
 *   <li>{@link org.fritzing.fritzing.Track#getSource <em>Source</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.fritzing.fritzing.FritzingPackage#getTrack()
 * @model extendedMetaData="name='Track' kind='empty'"
 * @generated
 */
public interface Track extends Connection, ITrackConnection {
	/**
	 * Returns the value of the '<em><b>Parent</b></em>' container reference.
	 * It is bidirectional and its opposite is '{@link org.fritzing.fritzing.Part#getTracks <em>Tracks</em>}'.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Parent</em>' container reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Parent</em>' container reference.
	 * @see #setParent(Part)
	 * @see org.fritzing.fritzing.FritzingPackage#getTrack_Parent()
	 * @see org.fritzing.fritzing.Part#getTracks
	 * @model opposite="tracks"
	 * @generated
	 */
	Part getParent();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Track#getParent <em>Parent</em>}' container reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Parent</em>' container reference.
	 * @see #getParent()
	 * @generated
	 */
	void setParent(Part value);

	/**
	 * Returns the value of the '<em><b>Target</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Target</em>' reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Target</em>' reference.
	 * @see #setTarget(ITrackConnection)
	 * @see org.fritzing.fritzing.FritzingPackage#getTrack_Target()
	 * @model keys="id" required="true"
	 *        extendedMetaData="kind='attribute' name='target'"
	 * @generated
	 */
	ITrackConnection getTarget();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Track#getTarget <em>Target</em>}' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Target</em>' reference.
	 * @see #getTarget()
	 * @generated
	 */
	void setTarget(ITrackConnection value);

	/**
	 * Returns the value of the '<em><b>Source</b></em>' reference.
	 * <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Source</em>' reference isn't clear,
	 * there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * @return the value of the '<em>Source</em>' reference.
	 * @see #setSource(ITrackConnection)
	 * @see org.fritzing.fritzing.FritzingPackage#getTrack_Source()
	 * @model keys="id" required="true"
	 *        extendedMetaData="kind='attribute' name='source'"
	 * @generated
	 */
	ITrackConnection getSource();

	/**
	 * Sets the value of the '{@link org.fritzing.fritzing.Track#getSource <em>Source</em>}' reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @param value the new value of the '<em>Source</em>' reference.
	 * @see #getSource()
	 * @generated
	 */
	void setSource(ITrackConnection value);

} // Track
