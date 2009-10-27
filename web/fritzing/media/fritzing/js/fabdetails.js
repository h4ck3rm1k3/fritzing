$(document).ready(function(){
	var stateSelect = $("#state")
	if(stateSelect) {
		stateSelect.change(function(){
			var old_state_text = $("#old_state_label").val()
			var old_state_val = $("#old_state_value").val()
			var text =
"This action will change the state of the order from '"+old_state_text+"' to '"+$("#state :selected").text()+
"'.\nDo you want to continue?"
			jConfirm(text, "State Change", function(r){
				if(r) {
					$("#form").submit()
				} else {
					$('#state option[value='+old_state_val+']').attr("selected","selected"); 
				}
			})
		});
	}
})

