$(document).ready(function(){
	$("#fzzFile").change(function() { fileChanged(this); });
});	
	
function fileChanged(theInput) {
	var fname = theInput.value;
	var ext = "fzz";
	if (fname.match(ext+"$") != ext ) {
		//alert("fname " + fname + " no match " + document.getElementById("upload_progress"));
		$("#upload_progress").text('Only ' + ext + ' files are allowed');
		return;
	}
	
	//theInput.parentNode.submit();			// doesn't work: you have to click the button!
	
	var thing = $("input[type='submit']", theInput.parentNode)
	if (thing != null) {
		thing.click();
	}
}

