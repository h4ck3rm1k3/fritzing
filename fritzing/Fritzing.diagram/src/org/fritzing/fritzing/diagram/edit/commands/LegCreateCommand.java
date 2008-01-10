/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.commands;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gmf.runtime.common.core.command.CommandResult;
import org.eclipse.gmf.runtime.emf.type.core.commands.CreateElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.ConfigureRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateRelationshipRequest;
import org.fritzing.fritzing.FritzingFactory;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.ILegConnection;
import org.fritzing.fritzing.Leg;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.diagram.edit.policies.FritzingBaseItemSemanticEditPolicy;

/**
 * @generated
 */
public class LegCreateCommand extends CreateElementCommand {

	/**
	 * @generated
	 */
	private final EObject source;

	/**
	 * @generated
	 */
	private final EObject target;

	/**
	 * @generated
	 */
	private Terminal container;

	/**
	 * @generated
	 */
	public LegCreateCommand(CreateRelationshipRequest request, EObject source,
			EObject target) {
		super(request);
		this.source = source;
		this.target = target;
		if (request.getContainmentFeature() == null) {
			setContainmentFeature(FritzingPackage.eINSTANCE.getTerminal_Leg());
		}

		// Find container element for the new link.
		// Climb up by containment hierarchy starting from the source
		// and return the first element that is instance of the container class.
		for (EObject element = source; element != null; element = element
				.eContainer()) {
			if (element instanceof Terminal) {
				container = (Terminal) element;
				super.setElementToEdit(container);
				break;
			}
		}
	}

	/**
	 * @generated
	 */
	public boolean canExecute() {
		if (source == null && target == null) {
			return false;
		}
		if (source != null && !(source instanceof ILegConnection)) {
			return false;
		}
		if (target != null && !(target instanceof ILegConnection)) {
			return false;
		}
		if (getSource() == null) {
			return true; // link creation is in progress; source is not defined yet
		}
		// target may be null here but it's possible to check constraint
		if (getContainer() == null) {
			return false;
		}
		return FritzingBaseItemSemanticEditPolicy.LinkConstraints
				.canCreateLeg_4003(getContainer(), getSource(), getTarget());
	}

	/**
	 * @generated
	 */
	protected EObject doDefaultElementCreation() {
		// org.fritzing.fritzing.Leg newElement = (org.fritzing.fritzing.Leg) super.doDefaultElementCreation();
		Leg newElement = FritzingFactory.eINSTANCE.createLeg();
		getContainer().setLeg(newElement);
		newElement.setSource(getSource());
		newElement.setTarget(getTarget());
		return newElement;
	}

	/**
	 * @generated
	 */
	protected EClass getEClassToEdit() {
		return FritzingPackage.eINSTANCE.getTerminal();
	}

	/**
	 * @generated
	 */
	protected CommandResult doExecuteWithResult(IProgressMonitor monitor,
			IAdaptable info) throws ExecutionException {
		if (!canExecute()) {
			throw new ExecutionException(
					"Invalid arguments in create link command"); //$NON-NLS-1$
		}
		return super.doExecuteWithResult(monitor, info);
	}

	/**
	 * @generated
	 */
	protected ConfigureRequest createConfigureRequest() {
		ConfigureRequest request = super.createConfigureRequest();
		request.setParameter(CreateRelationshipRequest.SOURCE, getSource());
		request.setParameter(CreateRelationshipRequest.TARGET, getTarget());
		return request;
	}

	/**
	 * @generated
	 */
	protected void setElementToEdit(EObject element) {
		throw new UnsupportedOperationException();
	}

	/**
	 * @generated
	 */
	protected ILegConnection getSource() {
		return (ILegConnection) source;
	}

	/**
	 * @generated
	 */
	protected ILegConnection getTarget() {
		return (ILegConnection) target;
	}

	/**
	 * @generated
	 */
	public Terminal getContainer() {
		return container;
	}
}
