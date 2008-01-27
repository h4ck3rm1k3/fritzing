/*
 * (c) Fachhochschule Potsdam
 */
package org.fritzing.fritzing.diagram.application;

import java.net.URL;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.eclipse.core.runtime.Platform;
import org.eclipse.equinox.app.IApplication;
import org.eclipse.equinox.app.IApplicationContext;
import org.eclipse.osgi.service.datalocation.Location;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.PlatformUI;
import org.fritzing.fritzing.diagram.part.FritzingDiagramEditorUtil;

/**
 * @generated
 */
public class FritzingApplication implements IApplication {

	/* (non-Javadoc)
	 * @see org.eclipse.equinox.app.IApplication#start(org.eclipse.equinox.app.IApplicationContext)
	 */
	public Object start(IApplicationContext context) throws Exception {
		// set workbench location to Fritzing user folder
		/*
		 * This is how it's supposed to be possible, see, e.g.
		 * http://dev.eclipse.org/newslists/news.eclipse.platform/msg63966.html
		 * Also remember to set the workspace to @noDefault in the run configuration.
		 * 
		 * In our case, however, there is some OSGI initializing going on before this
		 * point is reached. This initializing already _requires_ an existing workspace
		 * to save some data and therefore creates it automatically. At this point 
		 * it's then no longer possible to set the workspace to a different location.
		 * 
		 *  The only way to specify another workspace location is through a setting
		 *  in the config.ini:
		 *  osgi.instance.area.default=@user.home/Documents/Fritzing
		 *  This would be hardcoded, though, and also doesn't let us use the proper
		 *  standard location, which can only be found out by asking the registry. 
		 */
//		setWorkbenchDataLocation(
//				FritzingDiagramEditorUtil.getFritzingUserFolder().toURI().toURL());
		
		Display display = PlatformUI.createDisplay();
		try {
			int returnCode = PlatformUI.createAndRunWorkbench(display,
					new DiagramEditorWorkbenchAdvisor());
			if (returnCode == PlatformUI.RETURN_RESTART) {
				return IApplication.EXIT_RESTART;
			}
			return IApplication.EXIT_OK;
		} finally {
			display.dispose();
		}
	}

	/* (non-Javadoc)
	 * @see org.eclipse.equinox.app.IApplication#stop()
	 */
	public void stop() {
		final IWorkbench workbench = PlatformUI.getWorkbench();
		if (workbench == null)
			return;
		final Display display = workbench.getDisplay();
		display.syncExec(new Runnable() {
			public void run() {
				if (!display.isDisposed())
					workbench.close();
			}
		});
	}

	
	 public static void setWorkbenchDataLocation(URL url) {
		 /*
		 * Ascertain the existing location. If it is null, we can set it.
		 * When run from the Eclipse IDE, setting the workspace location in the launch configuration
		 * to "@noDefault" will set it as null.
		 */
		 Location instanceLocation = Platform.getInstanceLocation();
		 if(instanceLocation == null) {
			 Logger.getLogger(Logger.GLOBAL_LOGGER_NAME).log(Level.WARNING, 
					 "Instance Location is null, cannot set it in setWorkbenchDataLocation(URL)");
		 } else if (!instanceLocation.isSet()) {
			 instanceLocation.release();
			 /*
			 * If this is set to true, you can't run another instance
			 * of the app with this workspace open
			 */
			 instanceLocation.setURL(url, false);
		 } else {
			 Logger.getLogger(Logger.GLOBAL_LOGGER_NAME).log(Level.WARNING, 
			 	"Instance Location has already been set and cannot be changed");
		 }
	}
}
