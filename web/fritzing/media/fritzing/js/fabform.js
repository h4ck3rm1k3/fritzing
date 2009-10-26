String.prototype.startsWith = function(str) {return (this.match("^"+str)==str)}

$(document).ready(function(){
	loadForm()
	var manufact = $("#id_manufacturer")
	manufact.change(loadForm)
	manufact.addClass("required")
	
	toggleBillingAddress()
	$("#billing_enabled").change(toggleBillingAddress)
	syncShippingAndBillingValue()
	
	multifile();
	
	$("#fabform").validate({
		errorPlacement: function(error, element) {
			var elemName = element.attr("name");
			if (elemName.startsWith("billing") || elemName.startsWith("shipping") || elemName == "comments") {
				error.insertBefore(element);
			} else {
				error.insertAfter(element);
			}
		},
		rules: {
			confirm_email: {
				equalTo: "#email"
			},
			fritz_file: {
				accept: "fz|fzz"
			}
		},
		messages: {
			fritz_file: {
				accept: "Please enter a file with a valid extension (fz or fzz)"
			}
		}
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

var multifile = function() {
	$('input.upload').after('<div id="files_list"></div>');

	$("input.upload").change(function(){
		doIt(this);
	});

}	

var doIt = function doIt(obj) {
	$(obj).hide();
	$(obj).parent().prepend('<input type="file" class="upload" name="other_files[]" />')
		.find("input").change(function() {doIt(this)});
	var v = obj.value;
	if(v != '') {
		$("div#files_list").append('<div><a href="#none"><img src="/media/admin/img/admin/icon_deletelink.gif"/> '+v+'</a></div>')
			.find("a").click(function(){
				$(this).parent().remove();
				$(obj).remove();
				return true;
			});
	}
}

