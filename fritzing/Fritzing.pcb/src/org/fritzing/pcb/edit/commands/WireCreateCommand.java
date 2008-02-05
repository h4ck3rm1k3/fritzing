/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.pcb.edit.commands;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gmf.runtime.common.core.command.CommandResult;
import org.eclipse.gmf.runtime.emf.type.core.commands.CreateElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.ConfigureRequest;
import org.eclipse.gmf.runtime.emf.type.core.requests.CreateRelationshipRequest;
import org.fritzing.fritzing.Composite;
import org.fritzing.fritzing.FritzingFactory;
import org.fritzing.fritzing.FritzingPackage;
import org.fritzing.fritzing.IWireConnection;
import org.fritzing.fritzing.Wire;
import org.fritzing.pcb.edit.policies.FritzingBaseItemSemanticEditPolicy;
import org.fritzing.pcb.utils.UIDGenerator;

/**
 * @generated
 */
public class WireCreateCommand extends CreateElementCommand {

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
	private Composite container;

	/**
	 * @generated
	 */
	public WireCreateCommand(CreateRelationshipRequest request, EObject source,
			EObject target) {
		super(request);
		this.source = source;
		this.target = target;
		if (request.getContainmentFeature() == null) {
			setContainmentFeature(FritzingPackage.eINSTANCE
					.getComposite_Wires());
		}

		// Find container element for the new link.
		// Climb up by containment hierarchy starting from the source
		// and return the first element that is instance of the container class.
		for (EObject element = source; element != null; element = element
				.eContainer()) {
			if (element instanceof Composite) {
				container = (Composite) element;
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
		if (source != null && !(source instanceof IWireConnection)) {
			return false;
		}
		if (target != null && !(target instanceof IWireConnection)) {
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
				.canCreateWire_4001(getContainer(), getSource(), getTarget());
	}

	/**
	 * @generated NOT
	 */
	protected EObject doDefaultElementCreation() {
		// org.fritzing.fritzing.Wire newElement = (org.fritzing.fritzing.Wire) super.doDefaultElementCreation();
		Wire newElement = FritzingFactory.eINSTANCE.createWire();
		getContainer().getWires().add(newElement);
		newElement.setId(UIDGenerator.getInstance().genUID());
		newElement.setSource(getSource());
		newElement.setTarget(getTarget());
		return newElement;
	}

	/**
	 * @generated
	 */
	protected EClass getEClassToEdit() {
		return FritzingPackage.eINSTANCE.getComposite();
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
	protected IWireConnection getSource() {
		return (IWireConnection) source;
	}

	/**
	 * @generated
	 */
	protected IWireConnection getTarget() {
		return (IWireConnection) target;
	}

	/**
	 * @generated
	 */
	public Composite getContainer() {
		return container;
	}
}
