String.prototype.trim = function() { return this.replace(/^\s+|\s+$/, ''); };

$(document).ready(function(){
	$("form.search").submit(function(){
		var input = $("form.search > input[name=q]");
		if(input.val().trim()!= "") {
			input.val("site:fritzing.org "+input.val());
			return true;
		} else {
			input.val("")
			return false;
		}
	})
})