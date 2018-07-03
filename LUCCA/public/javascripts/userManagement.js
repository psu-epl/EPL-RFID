/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

$(function() {
  $('#search').trigger('click');
  $('#userNav').addClass("active");
});

$(function() {

  $('input[name="dateFilter"]').daterangepicker ({
    autoUpdateInput: false,
    locale: {
      cancelLabel: 'Clear'
    },
    ranges: {
      'Today': [moment(), moment()],
      'Yesterday': [moment().subtract(1, 'days'), moment().subtract(1, 'days')],
      'Last 7 Days': [moment().subtract(6, 'days'), moment()],
      'Last 30 Days': [moment().subtract(29, 'days'), moment()],
      'This Month': [moment().startOf('month'), moment().endOf('month')]
    }
  });

  $('input[name="dateFilter"]').on('apply.daterangepicker', function(ev, picker) {
    $(this).val(picker.startDate.format('MM/DD/YYYY') + ' - ' + picker.endDate.format('MM/DD/YYYY'));
  });

  $('input[name="dateFilter"]').on('hide.daterangepicker', function (ev, picker) {
    $('input[name="dateFilter"]').val('');
  });

  $('input[name="dateFilter"]').on('cancel.daterangepicker', function(ev, picker) {
    $(this).val('');
  });
});


// Attach a submit handler to the searchForm
$('#search').on("click", function(event) {

   // Stop form from submitting normally
   event.preventDefault();

  // Grab values from elements in searchForm:
  let $form = $('#searchForm'),
    name = $("#name").val(),
    dateRange = $("#dateRange").val(),
    status = $("#statusSelector").val(),
    badge = $("#badge").val(),
    station = $("#stationSelector").val(),
    url = '/userManagement';
  
  // Send the data using post
  let posting = $.post( url, {
    nameInput: name,
    dateFilter: dateRange,
    statusSelector: status,
    badgeInput: badge,
    stationSelector: station
  });

  // Put the results in a div
  posting.done(function(data) {
    $('tbody').empty();
    let tr = $('<tr></tr>'); //creates row
    let td = $('<td></td>'); //creates cell

    // Create a row for each user from POST and append elements
    $.each(data.users, function (index, user) {
      let row = tr.clone(); //deep copy
      let butt = $('<button class="btn btn-primary" onclick="fwd(this)" data-badge="' + user.badge + '">Edit details</button>');
      row.append(td.clone().text(user.first + " " + user.last));
      row.append(td.clone().text(user.badge));
      row.append(td.clone().text(user.confirmation));
      row.append(td.clone().append(butt));
      $('tbody').append(row);
    });
  });
});

$(".btn").mouseup(function(){
  $(this).blur();
})