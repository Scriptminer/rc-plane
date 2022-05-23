var lookupTable = [];

$(function() {
    setupDisplay();
    var conn = null;

    function log(msg, msgClass) {
        var control = $('#chat');
        control.html(control.html() +'<p>'+ msg + '</p><br/>');
        control.scrollTop(control.scrollTop() + 1000);
        console.log(msg);
    }

    function addLog(inMessage){
        console.log("ServerMsg: ");
        console.log(inMessage);
        var html = "<b>SERVER: </b>"+ inMessage; // Puts together the line of the message
        log(html);
    }

    function update_ui() {
      // Updates the status and changes the button to "connect" when disconnected, and vice versa
      var msg = '';

      if (false) { // PLACEHOLDER
        $('#status').text('disconnected');
        $('#connect').text('Connect');
      } else {
        $('#status').text('connected (' + conn.protocol + ')');
        $('#connect').text('Disconnect');
      }
    }

    function tick(){
        fetch("api/data").then(function(response){
            response.text().then(function(inString){
                updateDisplay(JSON.parse(inString));
                setTimeout(tock,150);
            });
        });
    }

    var allServerMessages = [];
    var latestMessageID = 0;

    function tock(){
        fetch("api/serverMessages").then(function(response){
            response.text().then(function(inString){
                var inData = inString.split("\n"); // Data comes in newline separated
                for(var i=0; i<inData.length; i++){
                    var id = parseInt(inData[i].substring(0,4)); // First four characters
                    var message = inData[i].substring(4); // Remaining characters
                    if(allServerMessages[id] == undefined){ // If this message is new
                        allServerMessages[id] = message; // Add this message to list of all received messages
                        console.log("SERVER MESSAGE: "+message);
                        document.getElementById("chat").innerHTML += "<b>MSG"+id+": </b>"+message+"<br>";
                    }

                }
                setTimeout(tick,150);
            });
        });
    }
    tick();
    //setTimeout(tick,5000); // TEMPORARY
    // Set page to fullscreen on doubleclick:
    $(document).dblclick(function(e){
        if($.fullscreen.isFullScreen()){
            $.fullscreen.exit();
        }else{
            $("body").fullscreen();
        }
    });

    var windowWidth = $(window).width();
    var windowHeight = $(window).height();

    $(window).resize(function(){ // When page is resized
        var newWidth = $(window).width();
        var newHeight = $(window).height();
        if(windowWidth == newWidth){ // If width has not changed (such as a mobile keyboard triggering the event)
            windowWidth = newWidth;
            windowHeight = newHeight;
            console.log("ABORTING RESIZE!");
            return;
        }
        console.log("windowWidth: "+ windowWidth +", newWidth: "+ newWidth);
        windowWidth = newWidth;
        windowHeight = newHeight;
        if(windowWidth/windowHeight < 1.1){
            alert("This window size is not recommended! (as you can probably tell!)");
        }
        var dataCopy = {};
        $.extend(dataCopy,data); // Deep copies the contents of data
        var logData = $('#chat').html();
        $("#displayGrid").html(""); // Deletes all contents of table
        setupDisplay(); // Re-creates table with new sizes
        data = dataCopy; // Restores data to its proper values (not the ones over-written by setupDisplay)
        $('#chat').html(logData);
        updateDisplay([]); // Updates the display
        console.log("Resizing Table");
    });

    updateDisplay([]);

});

var planeTitle = '<b><span style="color:#00d">BLUEBIRD v1</span> <span style="color:#0ad">Flight --001</span></b>';

//////////////////// DISPLAY ////////////////////

function setupDisplay(){

    fetch("static/DataTableTemplate.json").then(function(response){
        response.json().then(function(jsonTemplate){
            console.log("YOYO!");
            var tableData = jsonTemplate.table;

            var positions = [[],[],[]];

            for(var i=0;i<tableData.length;i++){ // Itterate through all sections in the display data table.
                var dataSection = tableData[i];
                var controlBox = document.createElement("div");
                controlBox.id = dataSection.id;
                controlBox.classList.add("controlBox");

                var heading = document.createElement("span");
                heading.classList.add("controlTitle");
                heading.innerHTML = dataSection.heading;

                controlBox.appendChild(heading);

                for(var j=0;j<dataSection.elements.length;j++){
                    var lineData = dataSection.elements[j];
                    var line = document.createElement("span");

                    if(lineData.linkSymbol == undefined){ // Normal line - there will only be one id tag
                        var lineHtml = lineData.name + ": <span id='"+ lineData.id +"' class='controlData'>---</span>"+lineData.symbol;
                        lookupTable.push(lineData.id);
                    }else{ // Dual tag line - there are two data fields, separated by a linking symbol on the same line
                        var lineHtml = lineData.name + ": <span id='"+ lineData.id[0] +"' class='controlData'>---</span>"+ lineData.symbol +" "+ lineData.linkSymbol +" <span id='"+ lineData.id[1] +"' class='controlData'>---</span>"+lineData.symbol;
                        lookupTable.push(lineData.id[0]);
                        lookupTable.push(lineData.id[1]);
                    }

                    line.innerHTML = "<br>"+lineHtml;
                    controlBox.appendChild(line);
                }

                var image = document.createElement("div");
                image.id = dataSection.id+"Img";
                image.classList.add("controlImg");
                image.style.backgroundImage = "url('static/controls/"+ dataSection.heading.toLowerCase().replace(" ","") +".svg')";

                controlBox.appendChild(image);

                positions[dataSection.position.row][dataSection.position.col] = {elem:controlBox,size:dataSection.size};
            }

            ///// TABLE POSITION SETUP /////
            var table = document.getElementById("displayGrid");
            var tableBody = document.createElement("tbody");
            table.appendChild(tableBody);

            for(var r=0;r<positions.length;r++){ // Fills in all cells
                var row = document.createElement("tr");
                tableBody.appendChild(row);
                for(var c=0;c<positions[r].length;c++){
                    if(positions[r][c]){
                      var cell = document.createElement("td");
                      cell.appendChild(positions[r][c].elem);
                      cell.rowSpan = positions[r][c].size.rowspan;
                      cell.colSpan = positions[r][c].size.colspan;
                      console.log("Adding:");
                      console.log(cell);
                      row.appendChild(cell);
                    }
                }
            }

            windowWidth = $(window).width();
            windowHeight = $(window).height();

            $("#displayGrid").width(windowWidth*0.95);
            $("#displayGrid").height(windowHeight*0.95);
            $("#displayGrid").css({"font-size": 95*(1/30)+"vh"});

            ///// CHAT SETUP /////
            var chat = document.createElement("div");
            chat.id = "chat";
            document.getElementById("logsBox").appendChild(chat);
            $(chat).css({"font-size": 95*(1/50)+"vh"});

            // Gives correct sizes to control images
            var width = 30;
            var heightMultiplier = 0.71782945736;
            for(var i=0;i<displayData.length;i++){
                var imgID = "#"+displayData[i].name+"Img";
                $(imgID).css({"width":width+"%"});
                $(imgID).css({"height":($(imgID).width()*heightMultiplier)+"px"});
            }

            $("#planeTitle").html(planeTitle);

            $(".nameInput").val(thisSender.name);
            $(".nameInput").css({"color":thisSender.colour});

            // Font Handling
            var elems = [{id:"#logTitle",size:0.1},{id:"#videoTitle",size:0.1},{id:"#chatform",size:0.05},{id:"#chat",size:0.05},{className:".controlTitle",size:0.16},{className:".controlBox",size:0.14},{className:".nameInputWrapper",size:0.05}];
            for(var i=0;i<elems.length;i++){
                var element = elems[i];
                if(element.id){
                    var parentHeight = $(element.id).parent().height();
                    $(element.id).css({"font-size":(parentHeight*element.size)+"px"});
                }
                if(element.className){
                    console.log("It's a class...");
                    var domElement = $(element.className)[0];
                    console.log(domElement);
                    var parentHeight = $(domElement).parent().height(); // Selects the first element to have this class
                    console.log("parent height is: "+ parentHeight);
                    $(element.className).css({"font-size":(parentHeight*element.size)+"px"});
                    console.log("setting font-size to "+ (parentHeight*element.size));
                }
            }
        });
    }).catch(console.error);
}

function updateDisplay(inData){
    for(var i=0; i<inData.length; i++){
        var datapoint = inData[i];
        $("#"+datapoint.id).html(datapoint.val);
        if(datapoint.warn == "1"){
            flashError(datapoint.id,true);
        }else{
            flashError(datapoint.id,false);
        }
    }
}

var flashingErrors = [];
var flashLength = 300;

function flashError(id,toggle){

    if(toggle == true){ // Turn on error-flashing
        if(flashingErrors[id] == undefined){ // If this id does not already have an error-flashng loop
            flashingErrors[id] = setInterval(function(){flashOn()},flashLength*2);
        }
    }else{ // Turn off error-flashing
        if(flashingErrors[id]){ // If there is an error-flashing loop to remove
            clearInterval(flashingErrors[id]);
            delete flashingErrors[id];
            setTimeout(function(){
                $("#"+id).closest(".controlBox").css("background-color","rgba(130,130,130,0.3)"); // Gets the parent control box
                $("#"+id).css("color","#000");
                },flashLength); // Resets colour after a pause
        }else{
        }
    }

    // Flashing Functions
    var flashOn = function(){
        $("#"+id).closest(".controlBox").css("background-color","rgba(180,0,0,0.3)"); // Flash the whole box red
        $("#"+id).css("color","rgba(180,255,0,1)"); // Flash text yellow
        setTimeout(flashOff,flashLength);
    };
    var flashOff = function(){
        $("#"+id).closest(".controlBox").css("background-color","rgba(180,180,0,0.3)"); // Flash the whole box yellow
        $("#"+id).css("color","rgba(180,0,0,1)"); // Flash text red
    };
}
