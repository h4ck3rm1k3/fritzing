/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.edit.parts;

import org.eclipse.draw2d.Connection;
import org.eclipse.draw2d.Graphics;
import org.eclipse.gef.EditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.ConnectionNodeEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles;
import org.eclipse.gmf.runtime.draw2d.ui.figures.PolylineConnectionEx;
import org.eclipse.gmf.runtime.draw2d.ui.figures.WrapLabel;
import org.eclipse.gmf.runtime.notation.View;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.widgets.Display;
import org.fritzing.fritzing.ITrackConnection;
import org.fritzing.fritzing.Part;
import org.fritzing.fritzing.Terminal;
import org.fritzing.fritzing.Track;
import org.fritzing.fritzing.diagram.edit.PartLoader;
import org.fritzing.fritzing.diagram.edit.PartLoaderRegistry;
import org.fritzing.fritzing.diagram.edit.policies.TrackItemSemanticEditPolicy;

/**
 * @generated
 */
public class TrackEditPart extends ConnectionFritzingEditPart {

	/**
	 * @generated
	 */
	public static final int VISUAL_ID = 4002;

	/**
	 * @generated NOT
	 */
	public boolean visible = false;

	/**
	 * @generated NOT
	 */
	public TrackEditPart(View view) {
		super(view);
		if (view.getElement() instanceof Track) {
			Track track = (Track) view.getElement();
			Part parent = track.getParent();
			if (parent != null) {
				PartLoader partLoader = PartLoaderRegistry.getInstance().get(
						parent.getGenus() + parent.getSpecies());
				if (partLoader != null) {
					Terminal source = (Terminal) track.getSource();
					Terminal target = (Terminal) track.getTarget();
					visible = partLoader.getTrackVisible(source.getId()
							+ target.getId());
				}
			}
		}
	}

	/**
	 * @generated
	 */
	protected void createDefaultEditPolicies() {
		super.createDefaultEditPolicies();
		installEditPolicy(EditPolicyRoles.SEMANTIC_ROLE,
				new TrackItemSemanticEditPolicy());
	}

	/**
	 * Creates figure for this edit part.
	 * 
	 * Body of this method does not depend on settings in generation model
	 * so you may safely remove <i>generated</i> tag and modify it.
	 * 
	 * @generated NOT
	 */

	protected Connection createConnectionFigure() {
		return new TrackFigure(visible);
	}

	/**
	 * @generated NOT
	 */
	public TrackFigure getPrimaryShape() {
		return (TrackFigure) getFigure();
	}

	/**
	 * @generated NOT
	 */
	public class TrackFigure extends ConnectionFritzingFigure {

		/**
		 * @generated NOT
		 */
		public TrackFigure() {
			this(false);
		}

		/**
		 * @generated NOT
		 */
		private boolean visible;

		/**
		 * @generated NOT
		 */
		public TrackFigure(boolean visible) {
			this.visible = visible;
			setLineWidth(connectionWidth = 1);
			createContents();
		}

		/**
		 * @generated NOT
		 */
		private void createContents() {
			if (!visible) {
				this.setVisible(false);
			}
		}

		public void paint(Graphics graphics) {
			if (this.visible) {
				super.paint(graphics);
			} else {
				this.setVisible(false);
			}
		}
	}

}
