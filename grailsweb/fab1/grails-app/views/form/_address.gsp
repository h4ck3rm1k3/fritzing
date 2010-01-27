

<table>
	<tbody>
		<tr class="prop">
			<td valign="top" class="name">
				<label for="firstName"><g:message code="address.firstName.label" default="First Name" /></label>
			</td>
			<td valign="top" class="value ${hasErrors(bean: address, field: 'firstName', 'errors')}">
				<g:textField name="${prefix}.firstName" value="${address?.firstName}" />
			</td>
		</tr>
		
		<tr class="prop">
			<td valign="top" class="name">
				<label for="lastName"><g:message code="address.lastName.label" default="Last Name" /></label>
			</td>
			<td valign="top" class="value ${hasErrors(bean: address, field: 'lastName', 'errors')}">
				<g:textField name="${prefix}.lastName" value="${address?.lastName}" />
			</td>
		</tr>
		
		<tr class="prop">
			<td valign="top" class="name">
				<label for="company"><g:message code="address.company.label" default="Company" /></label>
			</td>
			<td valign="top" class="value ${hasErrors(bean: address, field: 'company', 'errors')}">
				<g:textField name="${prefix}.company" value="${address?.company}" />
			</td>
		</tr>
		
		<tr class="prop">
			<td valign="top" class="name">
				<label for="street"><g:message code="address.street.label" default="Street" /></label>
			</td>
			<td valign="top" class="value ${hasErrors(bean: address, field: 'street', 'errors')}">
				<g:textField name="${prefix}.street" value="${address?.street}" />
			</td>
		</tr>
		
		<tr class="prop">
			<td valign="top" class="name">
				<label for="city"><g:message code="address.city.label" default="City" /></label>
			</td>
			<td valign="top" class="value ${hasErrors(bean: address, field: 'city', 'errors')}">
				<g:textField name="${prefix}.city" value="${address?.city}" />
			</td>
		</tr>
		
		<tr class="prop">
			<td valign="top" class="name">
				<label for="state"><g:message code="address.state.label" default="State" /></label>
			</td>
			<td valign="top" class="value ${hasErrors(bean: address, field: 'state', 'errors')}">
				<g:textField name="${prefix}.state" value="${address?.state}" />
			</td>
		</tr>
		
		<tr class="prop">
			<td valign="top" class="name">
				<label for="country"><g:message code="address.country.label" default="Country" /></label>
			</td>
			<td valign="top" class="value ${hasErrors(bean: address, field: 'country', 'errors')}">
				<g:textField name="${prefix}.country" value="${address?.country}" />
			</td>
		</tr>
		
		<tr class="prop">
			<td valign="top" class="name">
				<label for="zip"><g:message code="address.zip.label" default="Zip" /></label>
			</td>
			<td valign="top" class="value ${hasErrors(bean: address, field: 'zip', 'errors')}">
				<g:textField name="${prefix}.zip" value="${address?.zip}" />
			</td>
		</tr>
	</tbody>
</table>
