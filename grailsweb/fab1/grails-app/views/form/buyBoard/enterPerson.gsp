<%@ page import="com.g2one.fab1.Person" %>
<%@ page import="com.g2one.fab1.Address" %>
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<meta name="layout" content="main" />
		<g:set var="entityName" value="${message(code: 'person.label', default: 'Person')}" />
		<title><g:message code="default.create.label" args="[entityName]" /></title>
		<g:javascript src="jquery-1.4.js" />		
		<g:javascript src="enterPerson.js" />
	</head>
	<body>
		<div class="body">
			<h1><g:message code="default.create.label" args="[entityName]" /></h1>
			<div class="message">${order1.filename}: ${order1.qualityCheck ? 'perform' : 'no' } quality check; quantity:${order1.quantity}; totalPrice:<g:formatNumber number="${order1.totalPrice}" type="currency" currencyCode="EUR" /></div>
			
			<g:hasErrors bean="${person?.errors}">
				<div class="errors">
					<g:renderErrors bean="${person}" as="list" />
				</div>
			</g:hasErrors>
			<g:hasErrors bean="${shippingAddress?.errors}">
				<div class="errors">
					<g:renderErrors bean="${shippingAddress}" as="list" />
				</div>
			</g:hasErrors>
			<g:form action="buyBoard" method="post" >
				<div class="dialog">
					<table>
						<tbody>
							<tr class="prop">
								<td valign="top" class="name">
									<label for="email"><g:message code="person.email.label" default="Email" /></label>
								</td>
								<td valign="top" class="value ${hasErrors(bean: order1?.person, field: 'email', 'errors')}">
									<g:textField name="email" value="${order1?.person?.email}" />
								</td>
							</tr>
							<tr class="prop">
								<td valign="top" class="name">
									<label for="confirm email"><g:message code="person.email.label" default="Confirm Email" /></label>
								</td>
								<td valign="top" class="value ${hasErrors(bean: order1?.person, field: 'email', 'errors')}">
									<g:textField name="confirm email" value="${order1?.person?.email}" />
								</td>
							</tr>
							<tr>
								<td>
									<div class="shipping-customer-address">
										<span class="title">Shipping Address</span><br /> 
										<g:render template="address" model="['address':order1?.person?.shippingAddress, 'prefix':'shipping']" />
									</div>
								</td>
								<td>
									<div class="billing-customer-address">
										<g:checkBox name="billing_enabled" value="${false}" />
										<span class="title">Billing Address</span> <span class="help">Only if different from the shipping one</span>
										<g:render template="address" model="['address':order1?.person?.billingAddress, 'prefix':'billing']"/>
									</div>
								</td>
							</tr>
						</tbody>
					</table>
				</div>
				<div class="buttons">
					<span class="button"><g:submitButton name="person" class="save" value="${message(code: 'default.button.create.label', default: 'Create')}" /></span>
				</div>
			</g:form>
		</div>	
	</body>
</html>