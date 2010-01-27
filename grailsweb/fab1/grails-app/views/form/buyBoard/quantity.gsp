<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<meta name="layout" content="main" />
		<g:javascript src="jquery-1.4.js" />		
		<script type="text/javascript" >
			function getPricePerQualityCheck() {
				return ${application.getAttribute("pricePerQualityCheck")};
			}
			function getPricePerBoard() {
				return ${order1.pricePerBoard};
			}
			function getDestinations() {
				return ${application.getAttribute("destinationData")} 
			}
			
		</script>
		<g:javascript src="quantity.js" />
		<g:javascript src="fzzscript.js" />
	</head>
	<body>
		<div class="body">
			<g:render template="fzzupload" />
			<g:render template="boardDescription" />
			<g:form action="buyBoard">
				<div>
					<label for="quantity"><g:message code="order1.quantity" default="Quantity" /></label>
					<g:textField name="quantity" value="${order1?.quantity}" size="5" />
				</div>
				<div>
					<label for="qualityCheck"><g:message code="order1.qualityCheck" default="Quality Check" /></label>
					<g:checkBox name="qualityCheck" value="${order1?.qualityCheck}" />
				</div>
				<div>
					<label for="destination"><g:message code="order1.destination" default="Destination" /></label>
					<g:select name="destination" 
						value="${order1?.destination}" 
						optionKey = "name"
						optionValue = "name"
						from="${application.getAttribute('destinations')}" />     
				</div>
				<g:render template="calculatedPrices" />
				
				<g:hiddenField name="qualityCheckPrice" value="${order1.qualityCheckPrice}" />
				<g:hiddenField name="shipping" value="${order1.shipping}" />
				<g:hiddenField name="taxes" value="${order1.taxes}"  />
				<g:hiddenField name="totalPrice" value="${order1.totalPrice}" />
				<g:submitButton name="orderBoard" value="Order"/>
			</g:form>
			
		</div>
	</body>
</html>