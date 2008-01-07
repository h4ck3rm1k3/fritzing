/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.providers;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import java.util.Set;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.emf.common.util.Diagnostic;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.util.Diagnostician;
import org.eclipse.emf.transaction.TransactionalEditingDomain;
import org.eclipse.emf.transaction.util.TransactionUtil;
import org.eclipse.emf.validation.model.EvaluationMode;
import org.eclipse.emf.validation.model.IClientSelector;
import org.eclipse.emf.validation.model.IConstraintStatus;
import org.eclipse.emf.validation.service.IBatchValidator;
import org.eclipse.emf.validation.service.ITraversalStrategy;
import org.eclipse.emf.validation.service.ModelValidationService;
import org.eclipse.gmf.runtime.common.ui.services.action.contributionitem.AbstractContributionItemProvider;
import org.eclipse.gmf.runtime.common.ui.util.IWorkbenchPartDescriptor;
import org.eclipse.gmf.runtime.diagram.ui.OffscreenEditPartFactory;
import org.eclipse.gmf.runtime.diagram.ui.editparts.DiagramEditPart;
import org.eclipse.gmf.runtime.diagram.ui.parts.IDiagramWorkbenchPart;
import org.eclipse.gmf.runtime.emf.core.util.EMFCoreUtil;
import org.eclipse.gmf.runtime.notation.View;
import org.eclipse.jface.action.Action;
import org.eclipse.jface.action.IAction;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.IWorkbenchPart;
import org.eclipse.ui.PlatformUI;
import org.fritzing.fritzing.diagram.edit.parts.SketchEditPart;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorPlugin;
import org.fritzing.fritzing.diagram.part.FritzingVisualIDRegistry;
import org.fritzing.fritzing.diagram.part.ValidateAction;

/**
 * @generated
 */
public class FritzingValidationProvider extends
		AbstractContributionItemProvider {

	/**
	 * @generated
	 */
	private static boolean constraintsActive = false;

	/**
	 * @generated
	 */
	public static boolean shouldConstraintsBePrivate() {
		return false;
	}

	/**
	 * @generated
	 */
	protected IAction createAction(String actionId,
			IWorkbenchPartDescriptor partDescriptor) {
		if (ValidateAction.VALIDATE_ACTION_KEY.equals(actionId)) {
			return new ValidateAction(partDescriptor);
		}
		return super.createAction(actionId, partDescriptor);
	}

	/**
	 * @generated
	 */
	public static void runWithConstraints(View view, Runnable op) {
		final Runnable fop = op;
		Runnable task = new Runnable() {

			public void run() {
				try {
					constraintsActive = true;
					fop.run();
				} finally {
					constraintsActive = false;
				}
			}
		};
		TransactionalEditingDomain txDomain = TransactionUtil
				.getEditingDomain(view);
		if (txDomain != null) {
			try {
				txDomain.runExclusive(task);
			} catch (Exception e) {
				FritzingDiagramEditorPlugin.getInstance().logError(
						"Validation action failed", e); //$NON-NLS-1$
			}
		} else {
			task.run();
		}
	}

	/**
	 * @generated
	 */
	static boolean isInDefaultEditorContext(Object object) {
		if (shouldConstraintsBePrivate() && !constraintsActive) {
			return false;
		}
		if (object instanceof View) {
			return constraintsActive
					&& SketchEditPart.MODEL_ID.equals(FritzingVisualIDRegistry
							.getModelID((View) object));
		}
		return true;
	}

	/**
	 * @generated
	 */
	static final Map semanticCtxIdMap = new HashMap();

	/**
	 * @generated
	 */
	public static ITraversalStrategy getNotationTraversalStrategy(
			IBatchValidator validator) {
		return new CtxSwitchStrategy(validator);
	}

	/**
	 * @generated
	 */
	private static class CtxSwitchStrategy implements ITraversalStrategy {

		/**
		 * @generated
		 */
		private ITraversalStrategy defaultStrategy;

		/**
		 * @generated
		 */
		private String currentSemanticCtxId;

		/**
		 * @generated
		 */
		private boolean ctxChanged = true;

		/**
		 * @generated
		 */
		private EObject currentTarget;

		/**
		 * @generated
		 */
		private EObject preFetchedNextTarget;

		/**
		 * @generated
		 */
		CtxSwitchStrategy(IBatchValidator validator) {
			this.defaultStrategy = validator.getDefaultTraversalStrategy();
		}

		/**
		 * @generated
		 */
		public void elementValidated(EObject element, IStatus status) {
			defaultStrategy.elementValidated(element, status);
		}

		/**
		 * @generated
		 */
		public boolean hasNext() {
			return defaultStrategy.hasNext();
		}

		/**
		 * @generated
		 */
		public boolean isClientContextChanged() {
			if (preFetchedNextTarget == null) {
				preFetchedNextTarget = next();
				prepareNextClientContext(preFetchedNextTarget);
			}
			return ctxChanged;
		}

		/**
		 * @generated
		 */
		public EObject next() {
			EObject nextTarget = preFetchedNextTarget;
			if (nextTarget == null) {
				nextTarget = defaultStrategy.next();
			}
			this.preFetchedNextTarget = null;
			return this.currentTarget = nextTarget;
		}

		/**
		 * @generated
		 */
		public void startTraversal(Collection traversalRoots,
				IProgressMonitor monitor) {
			defaultStrategy.startTraversal(traversalRoots, monitor);
		}

		/**
		 * @generated
		 */
		private void prepareNextClientContext(EObject nextTarget) {
			if (nextTarget != null && currentTarget != null) {
				if (nextTarget instanceof View) {
					String id = ((View) nextTarget).getType();
					String nextSemanticId = id != null
							&& semanticCtxIdMap.containsKey(id) ? id : null;
					if ((currentSemanticCtxId != null && !currentSemanticCtxId
							.equals(nextSemanticId))
							|| (nextSemanticId != null && !nextSemanticId
									.equals(currentSemanticCtxId))) {
						this.ctxChanged = true;
					}
					currentSemanticCtxId = nextSemanticId;
				} else {
					// context of domain model
					this.ctxChanged = currentSemanticCtxId != null;
					currentSemanticCtxId = null;
				}
			} else {
				this.ctxChanged = false;
			}
		}
	}

	/**
	 * @generated
	 */
	static class JavaAudits {
	}
}
