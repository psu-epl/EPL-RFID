/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

$(function () {
  $('#userNav').addClass("active")
  
  $('#updateUser').on('click', function () {    
    let updatedData = {};

    $('input').each(function (index) {
      if ($(this).attr('type') === 'checkbox') {
        if ($(this).prop('checked') != ($(this).attr('data-initial') === 'true')) {
          updatedData['mailingList'] = $(this).prop('checked');
        }
      } else if ($(this).val().trim() !== $(this).attr('data-initial').trim()) {
        let key = $(this).prop('name');
        let value = $(this).val();
        updatedData[key] = value;
      }
    });

    if ($.isEmptyObject(updatedData)) {
      return;
    }

    let form = $('<form></form>')
      .attr('method', 'post')
      .attr('action', `/userManagement/updateUserInfo/${$('#badgeID').text()}`);

    $.each(updatedData, function (key, val) {
      let input = $('<input />')
        .attr('type', 'hidden')
        .attr('name', key)
        .attr('value', val);
      form.append(input);  
    });

    $(document.body).append(form);
    form.submit();
  });
});