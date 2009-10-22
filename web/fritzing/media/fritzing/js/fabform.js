$(document).ready(function(){
	loadForm()
	$("#id_manufacturer").change(loadForm)
	toggleBillingAddress()
	$("#billing_enabled").change(toggleBillingAddress)
	syncShippingAndBillingValue()
})

var loadForm = function() {
	var value = $("#id_manufacturer").val()
	if(value != '') {
		$.post(
			"/fab/manufacturer_form/"+value+"/"+Math.random(),
			cbGetForm,
			"html"
		);
	} else {
		cbGetForm("")
	}
}
	
var cbGetForm = function(data) {
	$("#manufacturer-form").html(data)
}

var toggleBillingAddress = function() {
	var checkbox = $("#billing_enabled");
	var checked = checkbox.is(':checked');
	enableBillingAddress(checked)
}

var enableBillingAddress = function(enabled) {
	$(".billing.customer-address > input").each(function() {
		if($(this).attr("name")) {
			if (!enabled) {
				$(this).attr('disabled', true);
				var value = "";
				var name = $(this).attr("name")
				name = name.substring(name.indexOf('-'))
				$(".shipping.customer-address > input[name$="+name+"]")
					.each(function() {
						value = $(this).val();
					})
				$(this).val(value)
			} else {
				$(this).removeAttr('disabled');
				$(this).val("")
			}   
		}
	})
}

var syncShippingAndBillingValue = function() {
	$(".shipping.customer-address > input").each(function() {
		var mirror = $(this) 
		$(this).change(function(){
			var name = mirror.attr("name")
			name = name.substring(name.indexOf('-'))
			$(".billing.customer-address > input[name$="+name+"]")
				.each(function() {
					if($(this).attr('disabled')) {
						$(this).val(mirror.val())
					}
				})
		})
	})
}