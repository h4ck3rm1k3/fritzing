/**
 * (c) Fachhochschule Potsdam
 *
 * $Id$
 */
package org.fritzing.fritzing.impl;

import org.eclipse.emf.common.notify.Notification;
import org.eclipse.emf.common.notify.NotificationChain;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.InternalEObject;

import org.eclipse.emf.ecore.impl.ENotificationImpl;

import org.eclipse.emf.ecore.util.EcoreUtil;

import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.Leg;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.Terminal;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Terminal</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.fritzing.fritzing.impl.TerminalImpl#getId <em>Id</em>}</li>
 *   <li>{@link org.fritzing.fritzing.impl.TerminalImpl#getName <em>Name</em>}</li>
 *   <li>{@link org.fritzing.fritzing.impl.TerminalImpl#getParent <em>Parent</em>}</li>
 *   <li>{@link org.fritzing.fritzing.impl.TerminalImpl#getLeg <em>Leg</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class TerminalImpl extends ElementImpl implements Terminal {
	/**
	 * The default value of the '{@link #getId() <em>Id</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getId()
	 * @generated
	 * @ordered
	 */
	protected static final String ID_EDEFAULT = null;

	/**
	 * The cached value of the '{@link #getId() <em>Id</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getId()
	 * @generated
	 * @ordered
	 */
	protected String id = ID_EDEFAULT;

	/**
	 * The default value of the '{@link #getName() <em>Name</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getName()
	 * @generated
	 * @ordered
	 */
	protected static final String NAME_EDEFAULT = null;

	/**
	 * The cached value of the '{@link #getName() <em>Name</em>}' attribute.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getName()
	 * @generated
	 * @ordered
	 */
	protected String name = NAME_EDEFAULT;

	/**
	 * The cached value of the '{@link #getLeg() <em>Leg</em>}' containment reference.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @see #getLeg()
	 * @generated
	 * @ordered
	 */
	protected Leg leg;

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	protected TerminalImpl() {
		super();
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	@Override
	protected EClass eStaticClass() {
		return FritzingPackage.Literals.TERMINAL;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public String getId() {
		return id;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public void setId(String newId) {
		String oldId = id;
		id = newId;
		if (eNotificationRequired())
			eNotify(new ENotificationImpl(this, Notification.SET, FritzingPackage.TERMINAL__ID, oldId, id));
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public String getName() {
		return name;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public void setName(String newName) {
		String oldName = name;
		name = newName;
		if (eNotificationRequired())
			eNotify(new ENotificationImpl(this, Notification.SET, FritzingPackage.TERMINAL__NAME, oldName, name));
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public Part getParent() {
		if (eContainerFeatureID != FritzingPackage.TERMINAL__PARENT) return null;
		return (Part)eContainer();
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public NotificationChain basicSetParent(Part newParent, NotificationChain msgs) {
		msgs = eBasicSetContainer((InternalEObject)newParent, FritzingPackage.TERMINAL__PARENT, msgs);
		return msgs;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public void setParent(Part newParent) {
		if (newParent != eInternalContainer() || (eContainerFeatureID != FritzingPackage.TERMINAL__PARENT && newParent != null)) {
			if (EcoreUtil.isAncestor(this, newParent))
				throw new IllegalArgumentException("Recursive containment not allowed for " + toString());
			NotificationChain msgs = null;
			if (eInternalContainer() != null)
				msgs = eBasicRemoveFromContainer(msgs);
			if (newParent != null)
				msgs = ((InternalEObject)newParent).eInverseAdd(this, FritzingPackage.PART__TERMINALS, Part.class, msgs);
			msgs = basicSetParent(newParent, msgs);
			if (msgs != null) msgs.dispatch();
		}
		else if (eNotificationRequired())
			eNotify(new ENotificationImpl(this, Notification.SET, FritzingPackage.TERMINAL__PARENT, newParent, newParent));
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public Leg getLeg() {
		return leg;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public NotificationChain basicSetLeg(Leg newLeg, NotificationChain msgs) {
		Leg oldLeg = leg;
		leg = newLeg;
		if (eNotificationRequired()) {
			ENotificationImpl notification = new ENotificationImpl(this, Notification.SET, FritzingPackage.TERMINAL__LEG, oldLeg, newLeg);
			if (msgs == null) msgs = notification; else msgs.add(notification);
		}
		return msgs;
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public void setLeg(Leg newLeg) {
		if (newLeg != leg) {
			NotificationChain msgs = null;
			if (leg != null)
				msgs = ((InternalEObject)leg).eInverseRemove(this, FritzingPackage.LEG__PARENT, Leg.class, msgs);
			if (newLeg != null)
				msgs = ((InternalEObject)newLeg).eInverseAdd(this, FritzingPackage.LEG__PARENT, Leg.class, msgs);
			msgs = basicSetLeg(newLeg, msgs);
			if (msgs != null) msgs.dispatch();
		}
		else if (eNotificationRequired())
			eNotify(new ENotificationImpl(this, Notification.SET, FritzingPackage.TERMINAL__LEG, newLeg, newLeg));
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	@Override
	public NotificationChain eInverseAdd(InternalEObject otherEnd, int featureID, NotificationChain msgs) {
		switch (featureID) {
			case FritzingPackage.TERMINAL__PARENT:
				if (eInternalContainer() != null)
					msgs = eBasicRemoveFromContainer(msgs);
				return basicSetParent((Part)otherEnd, msgs);
			case FritzingPackage.TERMINAL__LEG:
				if (leg != null)
					msgs = ((InternalEObject)leg).eInverseRemove(this, EOPPOSITE_FEATURE_BASE - FritzingPackage.TERMINAL__LEG, null, msgs);
				return basicSetLeg((Leg)otherEnd, msgs);
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
			case FritzingPackage.TERMINAL__PARENT:
				return basicSetParent(null, msgs);
			case FritzingPackage.TERMINAL__LEG:
				return basicSetLeg(null, msgs);
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
			case FritzingPackage.TERMINAL__PARENT:
				return eInternalContainer().eInverseRemove(this, FritzingPackage.PART__TERMINALS, Part.class, msgs);
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
			case FritzingPackage.TERMINAL__ID:
				return getId();
			case FritzingPackage.TERMINAL__NAME:
				return getName();
			case FritzingPackage.TERMINAL__PARENT:
				return getParent();
			case FritzingPackage.TERMINAL__LEG:
				return getLeg();
		}
		return super.eGet(featureID, resolve, coreType);
	}

	/**
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	@Override
	public void eSet(int featureID, Object newValue) {
		switch (featureID) {
			case FritzingPackage.TERMINAL__ID:
				setId((String)newValue);
				return;
			case FritzingPackage.TERMINAL__NAME:
				setName((String)newValue);
				return;
			case FritzingPackage.TERMINAL__PARENT:
				setParent((Part)newValue);
				return;
			case FritzingPackage.TERMINAL__LEG:
				setLeg((Leg)newValue);
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
			case FritzingPackage.TERMINAL__ID:
				setId(ID_EDEFAULT);
				return;
			case FritzingPackage.TERMINAL__NAME:
				setName(NAME_EDEFAULT);
				return;
			case FritzingPackage.TERMINAL__PARENT:
				setParent((Part)null);
				return;
			case FritzingPackage.TERMINAL__LEG:
				setLeg((Leg)null);
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
			case FritzingPackage.TERMINAL__ID:
				return ID_EDEFAULT == null ? id != null : !ID_EDEFAULT.equals(id);
			case FritzingPackage.TERMINAL__NAME:
				return NAME_EDEFAULT == null ? name != null : !NAME_EDEFAULT.equals(name);
			case FritzingPackage.TERMINAL__PARENT:
				return getParent() != null;
			case FritzingPackage.TERMINAL__LEG:
				return leg != null;
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
		result.append(" (id: ");
		result.append(id);
		result.append(", name: ");
		result.append(name);
		result.append(')');
		return result.toString();
	}

} //TerminalImpl
