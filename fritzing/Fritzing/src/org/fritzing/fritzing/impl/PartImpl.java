/**
 * (c) Fachhochschule Potsdam
 *
 * $Id$
 */
package org.fritzing.fritzing.impl;

import java.util.Collection;

import org.eclipse.emf.common.notify.Notification;
import org.eclipse.emf.common.notify.NotificationChain;

import org.eclipse.emf.common.util.EList;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.InternalEObject;

import org.eclipse.emf.ecore.impl.ENotificationImpl;

import org.eclipse.emf.ecore.util.EObjectContainmentWithInverseEList;
import org.eclipse.emf.ecore.util.EcoreUtil;
import org.eclipse.emf.ecore.util.InternalEList;

import org.fritzing.fritzing.Composite;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.Track;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Part</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.fritzing.fritzing.impl.PartImpl#getSpecies <em>Species</em>}</li>
 *   <li>{@link org.fritzing.fritzing.impl.PartImpl#getGenus <em>Genus</em>}</li>
 *   <li>{@link org.fritzing.fritzing.impl.PartImpl#getNote <em>Note</em>}</li>
 *   <li>{@link org.fritzing.fritzing.impl.PartImpl#getVersion <em>Version</em>}</li>
 *   <li>{@link org.fritzing.fritzing.impl.PartImpl#getFootprint <em>Footprint</em>}</li>
 *   <li>{@link org.fritzing.fritzing.impl.PartImpl#getTerminals <em>Terminals</em>}</li>
 *   <li>{@link org.fritzing.fritzing.impl.PartImpl#getParent <em>Parent</em>}</li>
 *   <li>{@link org.fritzing.fritzing.impl.PartImpl#getTracks <em>Tracks</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public abstract class PartImpl extends ElementImpl implements Part {
	/**
	 * The default value of the '{@link #getSpecies() <em>Species</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getSpecies()
	 * @generated
	 * @ordered
	 */
	protected static final String SPECIES_EDEFAULT = null;

	/**
	 * The cached value of the '{@link #getSpecies() <em>Species</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getSpecies()
	 * @generated
	 * @ordered
	 */
	protected String species = SPECIES_EDEFAULT;

	/**
	 * The default value of the '{@link #getGenus() <em>Genus</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getGenus()
	 * @generated
	 * @ordered
	 */
	protected static final String GENUS_EDEFAULT = null;

	/**
	 * The cached value of the '{@link #getGenus() <em>Genus</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getGenus()
	 * @generated
	 * @ordered
	 */
	protected String genus = GENUS_EDEFAULT;

	/**
	 * The default value of the '{@link #getNote() <em>Note</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getNote()
	 * @generated
	 * @ordered
	 */
	protected static final String NOTE_EDEFAULT = null;

	/**
	 * The cached value of the '{@link #getNote() <em>Note</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getNote()
	 * @generated
	 * @ordered
	 */
	protected String note = NOTE_EDEFAULT;

	/**
	 * The default value of the '{@link #getVersion() <em>Version</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getVersion()
	 * @generated
	 * @ordered
	 */
	protected static final String VERSION_EDEFAULT = null;

	/**
	 * The cached value of the '{@link #getVersion() <em>Version</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getVersion()
	 * @generated
	 * @ordered
	 */
	protected String version = VERSION_EDEFAULT;

	/**
	 * The default value of the '{@link #getFootprint() <em>Footprint</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getFootprint()
	 * @generated
	 * @ordered
	 */
	protected static final String FOOTPRINT_EDEFAULT = null;

	/**
	 * The cached value of the '{@link #getFootprint() <em>Footprint</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getFootprint()
	 * @generated
	 * @ordered
	 */
	protected String footprint = FOOTPRINT_EDEFAULT;

	/**
	 * The cached value of the '{@link #getTerminals() <em>Terminals</em>}' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getTerminals()
	 * @generated
	 * @ordered
	 */
	protected EList<Terminal> terminals;

	/**
	 * The cached value of the '{@link #getTracks() <em>Tracks</em>}' containment reference list.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getTracks()
	 * @generated
	 * @ordered
	 */
	protected EList<Track> tracks;

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	protected PartImpl() {
		super();
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	@Override
	protected EClass eStaticClass() {
		return FritzingPackage.Literals.PART;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public String getSpecies() {
		return species;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public void setSpecies(String newSpecies) {
		String oldSpecies = species;
		species = newSpecies;
		if (eNotificationRequired())
			eNotify(new ENotificationImpl(this, Notification.SET, FritzingPackage.PART__SPECIES, oldSpecies, species));
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public String getGenus() {
		return genus;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public void setGenus(String newGenus) {
		String oldGenus = genus;
		genus = newGenus;
		if (eNotificationRequired())
			eNotify(new ENotificationImpl(this, Notification.SET, FritzingPackage.PART__GENUS, oldGenus, genus));
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public String getNote() {
		return note;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public void setNote(String newNote) {
		String oldNote = note;
		note = newNote;
		if (eNotificationRequired())
			eNotify(new ENotificationImpl(this, Notification.SET, FritzingPackage.PART__NOTE, oldNote, note));
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public String getVersion() {
		return version;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public void setVersion(String newVersion) {
		String oldVersion = version;
		version = newVersion;
		if (eNotificationRequired())
			eNotify(new ENotificationImpl(this, Notification.SET, FritzingPackage.PART__VERSION, oldVersion, version));
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public String getFootprint() {
		return footprint;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public void setFootprint(String newFootprint) {
		String oldFootprint = footprint;
		footprint = newFootprint;
		if (eNotificationRequired())
			eNotify(new ENotificationImpl(this, Notification.SET, FritzingPackage.PART__FOOTPRINT, oldFootprint, footprint));
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public EList<Terminal> getTerminals() {
		if (terminals == null) {
			terminals = new EObjectContainmentWithInverseEList<Terminal>(Terminal.class, this, FritzingPackage.PART__TERMINALS, FritzingPackage.TERMINAL__PARENT);
		}
		return terminals;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public Composite getParent() {
		if (eContainerFeatureID != FritzingPackage.PART__PARENT) return null;
		return (Composite)eContainer();
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public NotificationChain basicSetParent(Composite newParent, NotificationChain msgs) {
		msgs = eBasicSetContainer((InternalEObject)newParent, FritzingPackage.PART__PARENT, msgs);
		return msgs;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public void setParent(Composite newParent) {
		if (newParent != eInternalContainer() || (eContainerFeatureID != FritzingPackage.PART__PARENT && newParent != null)) {
			if (EcoreUtil.isAncestor(this, newParent))
				throw new IllegalArgumentException("Recursive containment not allowed for " + toString());
			NotificationChain msgs = null;
			if (eInternalContainer() != null)
				msgs = eBasicRemoveFromContainer(msgs);
			if (newParent != null)
				msgs = ((InternalEObject)newParent).eInverseAdd(this, FritzingPackage.COMPOSITE__PARTS, Composite.class, msgs);
			msgs = basicSetParent(newParent, msgs);
			if (msgs != null) msgs.dispatch();
		}
		else if (eNotificationRequired())
			eNotify(new ENotificationImpl(this, Notification.SET, FritzingPackage.PART__PARENT, newParent, newParent));
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public EList<Track> getTracks() {
		if (tracks == null) {
			tracks = new EObjectContainmentWithInverseEList<Track>(Track.class, this, FritzingPackage.PART__TRACKS, FritzingPackage.TRACK__PARENT);
		}
		return tracks;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	@SuppressWarnings("unchecked")
	@Override
	public NotificationChain eInverseAdd(InternalEObject otherEnd, int featureID, NotificationChain msgs) {
		switch (featureID) {
			case FritzingPackage.PART__TERMINALS:
				return ((InternalEList<InternalEObject>)(InternalEList<?>)getTerminals()).basicAdd(otherEnd, msgs);
			case FritzingPackage.PART__PARENT:
				if (eInternalContainer() != null)
					msgs = eBasicRemoveFromContainer(msgs);
				return basicSetParent((Composite)otherEnd, msgs);
			case FritzingPackage.PART__TRACKS:
				return ((InternalEList<InternalEObject>)(InternalEList<?>)getTracks()).basicAdd(otherEnd, msgs);
		}
		return super.eInverseAdd(otherEnd, featureID, msgs);
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	@Override
	public NotificationChain eInverseRemove(InternalEObject otherEnd, int featureID, NotificationChain msgs) {
		switch (featureID) {
			case FritzingPackage.PART__TERMINALS:
				return ((InternalEList<?>)getTerminals()).basicRemove(otherEnd, msgs);
			case FritzingPackage.PART__PARENT:
				return basicSetParent(null, msgs);
			case FritzingPackage.PART__TRACKS:
				return ((InternalEList<?>)getTracks()).basicRemove(otherEnd, msgs);
		}
		return super.eInverseRemove(otherEnd, featureID, msgs);
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	@Override
	public NotificationChain eBasicRemoveFromContainerFeature(NotificationChain msgs) {
		switch (eContainerFeatureID) {
			case FritzingPackage.PART__PARENT:
				return eInternalContainer().eInverseRemove(this, FritzingPackage.COMPOSITE__PARTS, Composite.class, msgs);
		}
		return super.eBasicRemoveFromContainerFeature(msgs);
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	@Override
	public Object eGet(int featureID, boolean resolve, boolean coreType) {
		switch (featureID) {
			case FritzingPackage.PART__SPECIES:
				return getSpecies();
			case FritzingPackage.PART__GENUS:
				return getGenus();
			case FritzingPackage.PART__NOTE:
				return getNote();
			case FritzingPackage.PART__VERSION:
				return getVersion();
			case FritzingPackage.PART__FOOTPRINT:
				return getFootprint();
			case FritzingPackage.PART__TERMINALS:
				return getTerminals();
			case FritzingPackage.PART__PARENT:
				return getParent();
			case FritzingPackage.PART__TRACKS:
				return getTracks();
		}
		return super.eGet(featureID, resolve, coreType);
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	@SuppressWarnings("unchecked")
	@Override
	public void eSet(int featureID, Object newValue) {
		switch (featureID) {
			case FritzingPackage.PART__SPECIES:
				setSpecies((String)newValue);
				return;
			case FritzingPackage.PART__GENUS:
				setGenus((String)newValue);
				return;
			case FritzingPackage.PART__NOTE:
				setNote((String)newValue);
				return;
			case FritzingPackage.PART__VERSION:
				setVersion((String)newValue);
				return;
			case FritzingPackage.PART__FOOTPRINT:
				setFootprint((String)newValue);
				return;
			case FritzingPackage.PART__TERMINALS:
				getTerminals().clear();
				getTerminals().addAll((Collection<? extends Terminal>)newValue);
				return;
			case FritzingPackage.PART__PARENT:
				setParent((Composite)newValue);
				return;
			case FritzingPackage.PART__TRACKS:
				getTracks().clear();
				getTracks().addAll((Collection<? extends Track>)newValue);
				return;
		}
		super.eSet(featureID, newValue);
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	@Override
	public void eUnset(int featureID) {
		switch (featureID) {
			case FritzingPackage.PART__SPECIES:
				setSpecies(SPECIES_EDEFAULT);
				return;
			case FritzingPackage.PART__GENUS:
				setGenus(GENUS_EDEFAULT);
				return;
			case FritzingPackage.PART__NOTE:
				setNote(NOTE_EDEFAULT);
				return;
			case FritzingPackage.PART__VERSION:
				setVersion(VERSION_EDEFAULT);
				return;
			case FritzingPackage.PART__FOOTPRINT:
				setFootprint(FOOTPRINT_EDEFAULT);
				return;
			case FritzingPackage.PART__TERMINALS:
				getTerminals().clear();
				return;
			case FritzingPackage.PART__PARENT:
				setParent((Composite)null);
				return;
			case FritzingPackage.PART__TRACKS:
				getTracks().clear();
				return;
		}
		super.eUnset(featureID);
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	@Override
	public boolean eIsSet(int featureID) {
		switch (featureID) {
			case FritzingPackage.PART__SPECIES:
				return SPECIES_EDEFAULT == null ? species != null : !SPECIES_EDEFAULT.equals(species);
			case FritzingPackage.PART__GENUS:
				return GENUS_EDEFAULT == null ? genus != null : !GENUS_EDEFAULT.equals(genus);
			case FritzingPackage.PART__NOTE:
				return NOTE_EDEFAULT == null ? note != null : !NOTE_EDEFAULT.equals(note);
			case FritzingPackage.PART__VERSION:
				return VERSION_EDEFAULT == null ? version != null : !VERSION_EDEFAULT.equals(version);
			case FritzingPackage.PART__FOOTPRINT:
				return FOOTPRINT_EDEFAULT == null ? footprint != null : !FOOTPRINT_EDEFAULT.equals(footprint);
			case FritzingPackage.PART__TERMINALS:
				return terminals != null && !terminals.isEmpty();
			case FritzingPackage.PART__PARENT:
				return getParent() != null;
			case FritzingPackage.PART__TRACKS:
				return tracks != null && !tracks.isEmpty();
		}
		return super.eIsSet(featureID);
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	@Override
	public String toString() {
		if (eIsProxy()) return super.toString();

		StringBuffer result = new StringBuffer(super.toString());
		result.append(" (species: ");
		result.append(species);
		result.append(", genus: ");
		result.append(genus);
		result.append(", note: ");
		result.append(note);
		result.append(", version: ");
		result.append(version);
		result.append(", footprint: ");
		result.append(footprint);
		result.append(')');
		return result.toString();
	}

} //PartImpl
