$(document).ready(function(){
  $(".project_links input[name~=links_0]").example("Name");
  $(".project_links input[name~=links_1]").example("URL");
  $("#add_link").click(function() {
    new_link = '<p>'
      + '<input type="text" name="links_0" id="id_links_%name%" />'
      + '<input type="text" name="links_1" id="id_links_%url%" />'
      + '<img id="links_deleter_%count%" src="/media/admin/img/admin/icon_deletelink.gif" height="10" width="10" alt="X"/>'
      + '</p>';
    field_count = $(".project_links"
      ).children().filter("p").children().filter("input").length
    link_name_count = field_count+1;
    new_link = new_link.replace(/%name%/g, link_name_count);
    link_url_count = field_count+2;
    new_link = new_link.replace(/%url%/g, link_url_count);
    new_link = new_link.replace(/%count%/g, field_count);
    $(".project_links").append(new_link);
    $("#links_deleter_" + field_count).click(function() {
      $(this).parent().remove()
    });
    $("#id_links_" + link_name_count).example("Name");
    $("#id_links_" + link_url_count).example("URL");
  });
  $('#id_fritzing_files').MultiFile({ 
    list: '#fritzing_files_selection',
    STRING: {
       remove: '<img src="/media/admin/img/admin/icon_deletelink.gif" height="10" width="10" alt="X"/>'
      }
   });
  $('#id_main_image').MultiFile({ 
    list: '#main_image_selection',
    max: 1,
    STRING: {
       remove: '<img src="/media/admin/img/admin/icon_deletelink.gif" height="10" width="10" alt="X"/>'
      }
   });
   $('#id_other_images').MultiFile({
     list: '#other_images_selection',
     STRING: {
       remove: '<img src="/media/admin/img/admin/icon_deletelink.gif" height="10" width="10" alt="X"/>'
     },
     accept: 'gif|jpeg|jpg|png',
   });

  $('#id_other_files_1').MultiFile({
    list: '#other_files_selection',
    STRING: {
      file: '<input type="text" name="id_other_files_0" /> $file',
      remove: '<img src="/media/admin/img/admin/icon_deletelink.gif" height="10" width="10" alt="X"/>'
    },
    accept: 'gif|jpeg|jpg|png',
    afterFileSelect: function(element, value, master_element){
        $(".project_other_files input[name~=id_other_files_0]").example("Name");
    },
  });
  $('#id_description').autogrow();
  $('#id_instructions').autogrow();
});
