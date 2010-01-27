<div>
	<div>
		<label for="taxesLabel"><g:message code="order1.taxes" default="Taxes" /></label>
		<label name="taxesLabel" id="taxesLabel"><g:formatNumber number="${order1.taxes}" type="number" minFractionDigits="2" maxFractionDigits="2" /></label>
	</div>
	<div>
		<label for="shippingLabel"><g:message code="order1.shipping" default="Shipping" /></label>
		<label name="shippingLabel" id="shippingLabel"><g:formatNumber number="${order1.shipping}" type="number" minFractionDigits="2" maxFractionDigits="2"  /></label>
	</div>
	<div>
		<label for="qualityCheckPriceLabel"><g:message code="order1.qualityCheckPrice" default="Quality Check Cost" /></label>
		<label name="qualityCheckPriceLabel" id="qualityCheckPriceLabel"><g:formatNumber number="${order1.qualityCheckPrice}" type="number" minFractionDigits="2" maxFractionDigits="2"  /></label>
	</div>
	<div>
		<label for="totalPriceLabel"><g:message code="order1.totalPrice" default="Total Price" /></label>
		<label name="totalPriceLabel" id="totalPriceLabel"><g:formatNumber number="${order1.totalPrice}" type="currency" currencyCode="EUR" /></label>
	</div>
</div>
