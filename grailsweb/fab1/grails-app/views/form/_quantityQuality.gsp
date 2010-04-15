<div>
	<div>
		<label for="quantity"><g:message code="order1.quantity" default="Quantity" /></label>
		<label name="quantity" id="quantity">${order1?.quantity}</label>
	</div>
	<div>
		<label for="qualityCheck"><g:message code="order1.qualityCheck" default="Quality Check:" /></label>
		<label name="qualityCheck" id="qualityCheck">${order1?.qualityCheck ? "yes" : "no"}</label>
	</div>
	<div>
		<label for="destination"><g:message code="order1.destination" default="Destination" /></label>
		<label name="destination" id="destination">${order1?.destination}</label>
	</div>			
</div>			

