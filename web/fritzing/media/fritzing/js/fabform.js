String.prototype.startsWith = function(str) {return (this.match("^"+str)==str)}

$(document).ready(function(){
	loadForm()
	var manufact = $("#id_manufacturer")
	manufact.change(loadForm)
	manufact.addClass("required")
	
	toggleBillingAddress()
	$("#billing_enabled").change(toggleBillingAddress)
	syncShippingAndBillingValue()
	
	
	$("#fabform").validate({
		errorPlacement: function(error, element) {
			var elemName = element.attr("name");
			if (elemName.startsWith("billing") || elemName.startsWith("shipping")) {
				error.insertBefore(element);
			} else {
				error.insertAfter(element);
			}
		},
	});
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
			} else {
				$(this).removeAttr('disabled');
				$(this).val("")
			}   
		}
	})
	
	$(".billing.customer-address > span[class=required]").each(function() {
		if(enabled) {
			$(this).show()
		} else {
			$(this).hide()
		}
	})
	
	$(".billing.customer-address > label[class=error]").each(function() {
		$(this).remove()
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