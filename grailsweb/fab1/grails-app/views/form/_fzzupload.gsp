<div>
	<div class="${flash.uploadMessage ? 'message' : 'nomessage'}" id="upload_progress" name="upload_progress" >${flash.uploadMessage}</div> 
	<div id="content">
		<g:form action="buyBoard" enctype="multipart/form-data" >
			<input type="file" name="fzzFile" id="fzzFile" size="45"  />
						
			<g:if test="${order1?.totalPrice}">
				<div class="message">${order1.filename}</div>
			</g:if>

			<g:submitButton name="upload" value="whatever"  style="display:none" />
		</g:form>
	</div>
</div>
