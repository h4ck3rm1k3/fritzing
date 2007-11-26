/*
 * (c) Fachhochschule Potsdam
 */

package org.fritzing.fritzing.diagram.edit.policies;

import org.eclipse.gmf.runtime.diagram.ui.editpolicies.ComponentEditPolicy;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.requests.GroupRequest;

public class NonDeleteComponentEditPolicy extends
		ComponentEditPolicy {

	public NonDeleteComponentEditPolicy() {
		super();
	}
	
	 protected Command createDeleteSemanticCommand(GroupRequest deleteRequest)
	 {
		 return null; 
	 }

     protected Command createDeleteViewCommand(GroupRequest deleteRequest)
     {
        return null; 
     }	

}
