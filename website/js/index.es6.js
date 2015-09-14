
var derpdata = require("./derp.json");
var $ = require('jquery');
var LiarBot = require('liar_bot');


var lb = new LiarBot({
  name: 'ninja!',
});

console.log("liarbot, say your name!");
console.log(lb.toString());

console.log(derpdata);


function printOutput(outputText) {
    var po = $('#print_output');
    var line = $("<p></p>").text(outputText);
    po.append(line);

    $("#print_output").scrollTop($("#print_output")[0].scrollHeight);
}

printOutput("derp a little derpy!");


$('#btn_call').click((e) => {
  printOutput("User called!");
  
});

$('#btn_bid11').click((e) => {
  $('#btn_bid11').attr('disabled', true);
});

$('#btn_bid12').click((e) => {
  $('#print_output').css('background-color', 'grey');
  
});

