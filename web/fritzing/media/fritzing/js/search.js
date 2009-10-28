String.prototype.trim = function() { return this.replace(/^\s+|\s+$/, ''); };

$(document).ready(function(){
	googleSearch("search","fritzing.org");
	googleSearch("forumSearch","fritzing.org/forum");
})

var googleSearch = function(formClass,siteToSearch) {
	var formSelector = "form."+formClass 
	$(formSelector).submit(function(){
		var input = $(formSelector+" > input[name=q]");
		if(input.val().trim() != "") {
			input.val("site:"+siteToSearch+" "+input.val());
			return true;
		} else {
			input.val("")
			return false;
		}
	})
}