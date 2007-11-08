/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.part;

import org.eclipse.core.expressions.PropertyTester;
import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;
import org.fritzing.fritzing.FritzingPackage;

/**
 * @generated
 */
public class FritzingDomainModelElementTester extends PropertyTester {

	/**
	 * @generated
	 */
	public boolean test(Object receiver, String method, Object[] args,
			Object expectedValue) {
		if (false == receiver instanceof EObject) {
			return false;
		}
		EObject eObject = (EObject) receiver;
		EClass eClass = eObject.eClass();
		if (eClass == FritzingPackage.eINSTANCE.getDocumentRoot()) {
			return true;
		}
		if (eClass == FritzingPackage.eINSTANCE.getTerminal()) {
			return true;
		}
		if (eClass == FritzingPackage.eINSTANCE.getElement()) {
			return true;
		}
		if (eClass == FritzingPackage.eINSTANCE.getWire()) {
			return true;
		}
		if (eClass == FritzingPackage.eINSTANCE.getPart()) {
			return true;
		}
		if (eClass == FritzingPackage.eINSTANCE.getResistor()) {
			return true;
		}
		if (eClass == FritzingPackage.eINSTANCE.getLED()) {
			return true;
		}
		if (eClass == FritzingPackage.eINSTANCE.getButton()) {
			return true;
		}
		if (eClass == FritzingPackage.eINSTANCE.getComposite()) {
			return true;
		}
		if (eClass == FritzingPackage.eINSTANCE.getSketch()) {
			return true;
		}
		if (eClass == FritzingPackage.eINSTANCE.getBreadboard()) {
			return true;
		}
		if (eClass == FritzingPackage.eINSTANCE.getModule()) {
			return true;
		}
		if (eClass == FritzingPackage.eINSTANCE.getArduino()) {
			return true;
		}
		return false;
	}

}
