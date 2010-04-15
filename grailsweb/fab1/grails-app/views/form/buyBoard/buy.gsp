<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<meta name="layout" content="main" />
	</head>
	<body>
		<div class="body">
			<g:render template="boardDescription" />
			<g:render template="quantityQuality" />			
			<g:render template="calculatedPrices" />
			
			<g:form action="buyBoard">
				<div>
					<table>
						<tbody>
							<tr class="prop">
								<td valign="top" class="name">
									<label for="email"><g:message code="order1.email.label" default="Email" /></label>
								</td>
								<td valign="top" class="value ${hasErrors(bean: order1, field: 'email', 'errors')}">
									<g:textField name="email" value="${order1?.email}" />
								</td>
							</tr>
							<tr class="prop">
								<td valign="top" class="name">
									<label for="confirm email"><g:message code="person.email.label" default="Confirm Email" /></label>
								</td>
								<td valign="top" class="value ${hasErrors(bean: order1, field: 'email', 'errors')}">
									<g:textField name="confirm email" value="${order1?.email}" />
								</td>
							</tr>
						</tbody>
					</table>	
				</div>
				<input type="image" name="_eventId_paypal"  id="_eventId_paypal" src="https://www.paypal.com/en_US/DE/i/btn/btn_buynowCC_LG.gif" /> 
				
				%{-- <g:submitButton name="paypal" value="Buy"/>   --}%
				%{-- <g:actionSubmitImage event="paypal" src="https://www.paypal.com/en_US/DE/i/btn/btn_buynowCC_LG.gif" /> --}%
				
			</g:form>
			
		</div>
	</body>
</html>
