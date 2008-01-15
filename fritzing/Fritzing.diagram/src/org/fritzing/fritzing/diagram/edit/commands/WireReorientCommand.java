/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.commands;

import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.gmf.runtime.common.core.command.CommandResult;
import org.eclipse.gmf.runtime.emf.type.core.commands.EditElementCommand;
import org.eclipse.gmf.runtime.emf.type.core.requests.ReorientRelationshipRequest;
import org.fritzing.fritzing.Composite;
import org.fritzing.fritzing.IWireConnection;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.Wire;
import org.fritzing.fritzing.diagram.edit.policies.FritzingBaseItemSemanticEditPolicy;

/**
 * @generated
 */
public class WireReorientCommand extends EditElementCommand {

	/**
	 * @generated
	 */
	private final int reorientDirection;

	/**
	 * @generated
	 */
	private final EObject oldEnd;

	/**
	 * @generated
	 */
	private final EObject newEnd;

	/**
	 * @generated
	 */
	public WireReorientCommand(ReorientRelationshipRequest request) {
		super(request.getLabel(), request.getRelationship(), request);
		reorientDirection = request.getDirection();
		oldEnd = request.getOldRelationshipEnd();
		newEnd = request.getNewRelationshipEnd();
	}

	/**
	 * @generated
	 */
	public boolean canExecute() {
		if (!(getElementToEdit() instanceof Wire)) {
			return false;
		}
		if (reorientDirection == ReorientRelationshipRequest.REORIENT_SOURCE) {
			return canReorientSource();
		}
		if (reorientDirection == ReorientRelationshipRequest.REORIENT_TARGET) {
			return canReorientTarget();
		}
		return false;
	}

	/**
	 * @generated
	 */
	protected boolean canReorientSource() {
		if (!(oldEnd instanceof IWireConnection && newEnd instanceof IWireConnection)) {
			return false;
		}
		IWireConnection target = getLink().getTarget();
		if (!(getLink().eContainer() instanceof Composite)) {
			return false;
		}
		Composite container = (Composite) getLink().eContainer();
		return FritzingBaseItemSemanticEditPolicy.LinkConstraints
				.canExistWire_4001(container, getNewSource(), target);
	}

	/**
	 * @generated
	 */
	protected boolean canReorientTarget() {
		if (!(oldEnd instanceof IWireConnection && newEnd instanceof IWireConnection)) {
			return false;
		}
		IWireConnection source = getLink().getSource();
		if (!(getLink().eContainer() instanceof Composite)) {
			return false;
		}
		Composite container = (Composite) getLink().eContainer();
		return FritzingBaseItemSemanticEditPolicy.LinkConstraints
				.canExistWire_4001(container, source, getNewTarget());
	}

	/**
	 * @generated
	 */
	protected CommandResult doExecuteWithResult(IProgressMonitor monitor,
			IAdaptable info) throws ExecutionException {
		if (!canExecute()) {
			throw new ExecutionException(
					"Invalid arguments in reorient link command"); //$NON-NLS-1$
		}
		if (reorientDirection == ReorientRelationshipRequest.REORIENT_SOURCE) {
			return reorientSource();
		}
		if (reorientDirection == ReorientRelationshipRequest.REORIENT_TARGET) {
			return reorientTarget();
		}
		throw new IllegalStateException();
	}

	/**
	 * @generated
	 */
	protected CommandResult reorientSource() throws ExecutionException {
		getLink().setSource(getNewSource());
		return CommandResult.newOKCommandResult(getLink());
	}

	/**
	 * @generated
	 */
	protected CommandResult reorientTarget() throws ExecutionException {
		getLink().setTarget(getNewTarget());
		return CommandResult.newOKCommandResult(getLink());
	}

	/**
	 * @generated
	 */
	protected Wire getLink() {
		return (Wire) getElementToEdit();
	}

	/**
	 * @generated
	 */
	protected IWireConnection getOldSource() {
		return (IWireConnection) oldEnd;
	}

	/**
	 * @generated
	 */
	protected IWireConnection getNewSource() {
		return (IWireConnection) newEnd;
	}

	/**
	 * @generated
	 */
	protected IWireConnection getOldTarget() {
		return (IWireConnection) oldEnd;
	}

	/**
	 * @generated
	 */
	protected IWireConnection getNewTarget() {
		return (IWireConnection) newEnd;
	}
}
