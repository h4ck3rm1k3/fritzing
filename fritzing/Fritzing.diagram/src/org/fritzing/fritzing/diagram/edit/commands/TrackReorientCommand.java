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
import org.fritzing.fritzing.ITrackConnection;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.Track;
import org.fritzing.fritzing.diagram.edit.policies.FritzingBaseItemSemanticEditPolicy;

/**
 * @generated
 */
public class TrackReorientCommand extends EditElementCommand {

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
	public TrackReorientCommand(ReorientRelationshipRequest request) {
		super(request.getLabel(), request.getRelationship(), request);
		reorientDirection = request.getDirection();
		oldEnd = request.getOldRelationshipEnd();
		newEnd = request.getNewRelationshipEnd();
	}

	/**
	 * @generated
	 */
	public boolean canExecute() {
		if (!(getElementToEdit() instanceof Track)) {
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
		if (!(oldEnd instanceof ITrackConnection && newEnd instanceof ITrackConnection)) {
			return false;
		}
		ITrackConnection target = getLink().getTarget();
		if (!(getLink().eContainer() instanceof Part)) {
			return false;
		}
		Part container = (Part) getLink().eContainer();
		return FritzingBaseItemSemanticEditPolicy.LinkConstraints
				.canExistTrack_4002(container, getNewSource(), target);
	}

	/**
	 * @generated
	 */
	protected boolean canReorientTarget() {
		if (!(oldEnd instanceof ITrackConnection && newEnd instanceof ITrackConnection)) {
			return false;
		}
		ITrackConnection source = getLink().getSource();
		if (!(getLink().eContainer() instanceof Part)) {
			return false;
		}
		Part container = (Part) getLink().eContainer();
		return FritzingBaseItemSemanticEditPolicy.LinkConstraints
				.canExistTrack_4002(container, source, getNewTarget());
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
	protected Track getLink() {
		return (Track) getElementToEdit();
	}

	/**
	 * @generated
	 */
	protected ITrackConnection getOldSource() {
		return (ITrackConnection) oldEnd;
	}

	/**
	 * @generated
	 */
	protected ITrackConnection getNewSource() {
		return (ITrackConnection) newEnd;
	}

	/**
	 * @generated
	 */
	protected ITrackConnection getOldTarget() {
		return (ITrackConnection) oldEnd;
	}

	/**
	 * @generated
	 */
	protected ITrackConnection getNewTarget() {
		return (ITrackConnection) newEnd;
	}
}
