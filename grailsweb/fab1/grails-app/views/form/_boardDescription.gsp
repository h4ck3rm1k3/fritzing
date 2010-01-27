<div>Board Dimensions: 
	<div>width:<g:formatNumber number="${order1.width}" type="number" maxFractionDigits="2" />mm </div>
	<div>height:<g:formatNumber number="${order1.height}" type="number" maxFractionDigits="2" />mm </div>
	<div>area:<g:formatNumber number="${order1.height * order1.width}" type="number" maxFractionDigits="2" /> sq mm</div>
</div>
<div>Price per board: <g:formatNumber number="${order1.pricePerBoard}" type="currency" currencyCode="EUR" /></div>
