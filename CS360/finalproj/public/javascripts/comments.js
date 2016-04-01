$(document).ready(function(){
    $("#serialize").click(function(){
        console.log('Serialize Button Pressed');
        var twinName = $("#Name").val().split("").reverse().join("");
        var myobj = {Name:$("#Name").val(),Comment:$("#Comment").val(),EvilTwin:twinName};
        jobj = JSON.stringify(myobj);
        $("#json").text(jobj);

        var url = "comment";
        $.ajax({
          url:url,
          type: "POST",
          data: jobj,
          contentType: "application/json; charset=utf-8",
          success: function(data,textStatus) {
                $("#done").html(textStatus);
          }
        })
    });

    $("#getThem").click(function() {
      console.log('Get Them Button Pressed');
      $.getJSON('comment', function(data) {
        console.log(data);
        var everything = "<ul>";
        for(var comment in data) {
          com = data[comment];
          everything += "<li>Name: " + com.Name + " -- Comment: " + com.Comment + " -- Evil Twin: " + com.EvilTwin + "</li>";
        }
        everything += "</ul>";
        $("#comments").html(everything);
      });
    });

    $("#getTwins").click(function() {
        $.getJSON('twins', function(result) {
        console.log(result);
        var everything = "<ul>";
        for(var twin in result) {
          t = result[twin];
          everything += "<li>Evil Name: " + t.Name + " -- Evil Comment: " + t.Comment + "</li>";
        }
        everything += "</ul>";
        $("#twins").html(everything);
        });
    });
});

