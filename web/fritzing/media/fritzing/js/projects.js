$(document).ready(function(){
  field_count = $(".project_links p input[type~=text]").length/2;
  
  if(field_count > 0) {
	for(i=0;i<field_count;i++){
	  $("#links_deleter_" + i).click(function() {
        $(this).parent().remove()
      });
    }
  }
  if(!prev_resources) {
	$("#id_links_title_0").example("Name");
	$("#id_links_url_0").example("URL");
  }
  
  $("#add_link").click(function() {
    new_link = '<p>'
      + '<input type="text" name="links_title" id="id_links_title_%name%" class="title" />'
      + '<input type="text" name="links_url" id="id_links_url_%url%" class="url" />'
      + '<img id="links_deleter_%count%" src="/media/admin/img/admin/icon_deletelink.gif" height="10" width="10" alt="X"/>'
      + '</p>';
    field_count = $(".project_links p input[type~=text]").length/2;
    
    new_link = new_link.replace(/%name%/g, field_count);
    new_link = new_link.replace(/%url%/g, field_count);
    new_link = new_link.replace(/%count%/g, field_count);
    
    $(".project_links").append(new_link);
    
    $("#links_deleter_" + field_count).click(function() {
      $(this).parent().remove()
    });
    
    $("#id_links_title_" + field_count).example("Name");
    $("#id_links_url_" + field_count).example("URL");
  });
  
  $('#id_fritzing_files').MultiFile({ 
    list: '#fritzing_files_selection',
    accept: 'fz|fzz|fzp|fzpz|fzb|fzbz',
    STRING: {
       remove: '<img src="/media/admin/img/admin/icon_deletelink.gif" height="10" width="10" alt="X"/>'
      }
   });
  $('#id_main_image').MultiFile({ 
    list: '#main_image_selection',
    max: 1,
    STRING: {
       remove: '<img src="/media/admin/img/admin/icon_deletelink.gif" height="10" width="10" alt="X"/>'
    },
  	accept: 'gif|jpeg|jpg|png|tiff|tif',
   });
   $('#id_other_images').MultiFile({
     list: '#other_images_selection',
     STRING: {
       remove: '<img src="/media/admin/img/admin/icon_deletelink.gif" height="10" width="10" alt="X"/>'
     },
     accept: 'gif|jpeg|jpg|png|tiff|tif',
   });

  $('#id_code').MultiFile({
    list: '#code_selection',
    STRING: {
      remove: '<img src="/media/admin/img/admin/icon_deletelink.gif" height="10" width="10" alt="X"/>'
    },
    afterFileSelect: function(element, value, master_element){
        $(".project_code input[name~=id_code]").example("Name");
    },
  });
  
  $('#id_examples').MultiFile({
    list: '#examples_selection',
    STRING: {
      remove: '<img src="/media/admin/img/admin/icon_deletelink.gif" height="10" width="10" alt="X"/>'
    },
    afterFileSelect: function(element, value, master_element){
        $(".project_examples input[name~=id_examples]").example("Name");
    },
  });
  
  $('#id_description').autogrow();
  $('#id_instructions').autogrow();
  
  try {
	  if(main_image_already_loaded) {
		  $('#id_main_image').attr('disabled', 'disabled')
	  }
  } catch(e) {}
});


function file_deleted(field_name,value,index) {
	$('#'+field_name+"_"+index).remove()
	var input = $('<input type="hidden"></input>')
		.attr('name','deleted_'+field_name)
		.attr('value',value);

	$('#my_form').append(input);
}
