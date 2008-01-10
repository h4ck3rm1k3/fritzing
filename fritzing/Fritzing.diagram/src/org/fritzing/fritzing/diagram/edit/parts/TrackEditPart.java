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
public class TrackEditPart extends ConnectionNodeEditPart {

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
	 * @generated
	 */
	protected boolean addFixedChild(EditPart childEditPart) {
		if (childEditPart instanceof TrackNameEditPart) {
			((TrackNameEditPart) childEditPart).setLabel(getPrimaryShape()
					.getFigureTrackNameFigure());
			return true;
		}
		return false;
	}

	/**
	 * @generated
	 */
	protected void addChildVisual(EditPart childEditPart, int index) {
		if (addFixedChild(childEditPart)) {
			return;
		}
		super.addChildVisual(childEditPart, -1);
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
	 * @generated
	 */
	public TrackFigure getPrimaryShape() {
		return (TrackFigure) getFigure();
	}

	/**
	 * @generated
	 */
	public class TrackFigure extends PolylineConnectionEx {

		/**
		 * @generated
		 */
		private WrapLabel fFigureTrackNameFigure;

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
			createContents();
		}

		/**
		 * @generated NOT
		 */
		private void createContents() {

			fFigureTrackNameFigure = new WrapLabel();
			fFigureTrackNameFigure.setText("..");

			fFigureTrackNameFigure.setFont(FFIGURETRACKNAMEFIGURE_FONT);

			this.add(fFigureTrackNameFigure);

			if (!visible) {
				this.setVisible(false);
			}
		}

		/**
		 * @generated
		 */
		public WrapLabel getFigureTrackNameFigure() {
			return fFigureTrackNameFigure;
		}

		public void paint(Graphics graphics) {
			if (this.visible) {
				super.paint(graphics);
			} else {
				this.setVisible(false);
			}
		}
	}

	/**
	 * @generated
	 */
	static final Font FFIGURETRACKNAMEFIGURE_FONT = new Font(Display
			.getCurrent(),
			Display.getDefault().getSystemFont().getFontData()[0].getName(), 8,
			SWT.NORMAL);

}
