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
import org.fritzing.fritzing.ILegConnection;
import org.fritzing.fritzing.Leg;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.diagram.edit.policies.FritzingBaseItemSemanticEditPolicy;

/**
 * @generated
 */
public class LegReorientCommand extends EditElementCommand {

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
	public LegReorientCommand(ReorientRelationshipRequest request) {
		super(request.getLabel(), request.getRelationship(), request);
		reorientDirection = request.getDirection();
		oldEnd = request.getOldRelationshipEnd();
		newEnd = request.getNewRelationshipEnd();
	}

	/**
	 * @generated
	 */
	public boolean canExecute() {
		if (!(getElementToEdit() instanceof Leg)) {
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
		if (!(oldEnd instanceof ILegConnection && newEnd instanceof ILegConnection)) {
			return false;
		}
		ILegConnection target = getLink().getTarget();
		if (!(getLink().eContainer() instanceof Terminal)) {
			return false;
		}
		Terminal container = (Terminal) getLink().eContainer();
		return FritzingBaseItemSemanticEditPolicy.LinkConstraints
				.canExistLeg_4003(container, getNewSource(), target);
	}

	/**
	 * @generated
	 */
	protected boolean canReorientTarget() {
		if (!(oldEnd instanceof ILegConnection && newEnd instanceof ILegConnection)) {
			return false;
		}
		ILegConnection source = getLink().getSource();
		if (!(getLink().eContainer() instanceof Terminal)) {
			return false;
		}
		Terminal container = (Terminal) getLink().eContainer();
		return FritzingBaseItemSemanticEditPolicy.LinkConstraints
				.canExistLeg_4003(container, source, getNewTarget());
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
	protected Leg getLink() {
		return (Leg) getElementToEdit();
	}

	/**
	 * @generated
	 */
	protected ILegConnection getOldSource() {
		return (ILegConnection) oldEnd;
	}

	/**
	 * @generated
	 */
	protected ILegConnection getNewSource() {
		return (ILegConnection) newEnd;
	}

	/**
	 * @generated
	 */
	protected ILegConnection getOldTarget() {
		return (ILegConnection) oldEnd;
	}

	/**
	 * @generated
	 */
	protected ILegConnection getNewTarget() {
		return (ILegConnection) newEnd;
	}
}
