$(document).ready(function(){
	var stateSelect = $("#state")
	if(stateSelect) {
		stateSelect.change(function(){
			$("#form").submit()
		});
	}
})

