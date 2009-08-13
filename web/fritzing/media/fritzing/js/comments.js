function validateComment() {
	if(trim($("#id_comment").val()) == "") {
		err = $("#required_error"); 
		err.removeAttr("style")
		err.focus()
		return false;
	}
	return true;
}

//Removes leading whitespaces
function LTrim( value ) {
	
	var re = /\s*((\S+\s*)*)/;
	return value.replace(re, "$1");
	
}

// Removes ending whitespaces
function RTrim( value ) {
	
	var re = /((\s*\S+)*)\s*/;
	return value.replace(re, "$1");
	
}

// Removes leading and ending whitespaces
function trim( value ) {
	
	return LTrim(RTrim(value));
	
}
