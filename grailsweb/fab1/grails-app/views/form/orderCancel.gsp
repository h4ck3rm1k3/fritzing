<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<meta name="layout" content="main" />
	</head>
	<body>
		<div class="body">
			ORDER CANCEL
		</div>
		<div>
			<label for="quantity"><g:message code="order1.quantity" default="Quantity" /></label>
			<label name="quantity" >${order?.quantity}</label>
		</div>
		<div>
			<label for="qualityCheck"><g:message code="order1.qualityCheck" default="Quality Check" /></label>
			<label name="qualityCheck" >${order?.qualityCheck ? " yes" : " no"}</label>
		</div>
		<div>
			<label for="destination"><g:message code="order1.destination" default="Destination" /></label>
			<label name="destination" >${order?.destination}</label>
		</div>
		<div>
			<label for="email"><g:message code="order1.email" default="Email" /></label>
			<label name="email" >${order?.email}</label>
		</div>
		<div>
			<label for="total price"><g:message code="order1.totalPrice" default="Total Price" /></label>
			<label name="total price" >${order?.totalPrice}</label>
		</div>
	</body>
</html>