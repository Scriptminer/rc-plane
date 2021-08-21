
$(function() {
    setupDisplay();
    var conn = null;

    function log(msg, msgClass) {
      var control = $('#chat');
      control.html(control.html() +'<span class="'+ msgClass +'">'+ msg + '</span><br/>');
      control.scrollTop(control.scrollTop() + 1000);
      console.log(msg);
    }

    function connect() {
      disconnect();

      var transports = ["websocket", "xhr-streaming", "iframe-eventsource", "iframe-htmlfile", "xhr-polling", "iframe-xhr-polling", "jsonp-polling"];

      conn = new SockJS('http://' + window.location.host + '/chat', transports);

      log('Connecting to server...',"serverMsgYellow");

      conn.onopen = function() {
        // Called when we manage to connect to the server.
        log('Connected to server!',"serverMsgGreen");
        update_ui();
      };

      conn.onmessage = function(e) {
        // Called when we receive a message from the server.
        try {
          o = JSON.parse(e.data);
        } catch (exn) {
          log('Received corrupt data packet (' + exn + '): ' + e.data, "serverMsgRed");
          return;
        }

        if (!o.type) {
          log('Received packet with no type: ' + e.data, "serverMsgRed");
          return;
        }

        console.log(o);

        switch(o.type){
            case "chat":
                addChat(o.data.msg,o.data.sender);
                break;
            case "serverMsg":
                var serverMsg = o.data;
                addChat(serverMsg,{name:"SERVER",colour:"#000"},true); // Sends the message, but as the name server
                console.log(serverMsg);
                break;
            case "data":
                updateDisplay(o.data);
                break;
            case "warning":
                for(var i=0;i<o.data.length;i++){ // Goes through the array
                    flashError(o.data[i].element,o.data[i].toggle); // Sets different elements to flash
                }
                break;
        }
      };

      conn.onclose = function() {
        // Called when we are disconnected from the server.
        log('It appears you have been disconnected!<br>',"serverMsgRed");
        conn = null;
        update_ui();
      };

    }

    function addChat(inMessage,sender,server){
        var sender = JSON.parse(JSON.stringify(sender)); // Deep copies the sender details
        if(!inMessage){
            return;
        }
        if(sender.name == thisSender.name){ // If this is this device's message
            sender.className = "thisSender";
        }else{
            sender.className = sender.name;
        }

        if(server){
            var message = jQuery('<p>' + inMessage + ' </p>').html(); // Allows html tags in server messages
            console.log("ServerMsg: ");
            console.log(inMessage);
        }else{
            var message = jQuery('<p>' + inMessage + ' </p>').text(); // Converts the html to plain text
        }

        var html = "<span class='name "+ sender.className +"' style='color:"+ sender.colour +"'>"+ sender.name +": </span>"+ message; // Puts together the line of the message
        log(html);
    }

    // Example call of addChat: addChat("Hello everyone! I'm a random bot secretly controlled by Aidan!",{colour:"#dab",name:"The Bot"})

    function disconnect() {
      // Called to request disconnection from the server.
      if (conn != null) {
        log('Disconnecting from the server...',"serverMsgYellow");

        conn.close();
        conn = null;

        update_ui();
      }
    }

    function update_ui() {
      // Updates the status and changes the button to "connect" when disconnected, and vice versa
      var msg = '';

      if (conn == null || conn.readyState != SockJS.OPEN) {
        $('#status').text('disconnected');
        $('#connect').text('Connect');
      } else {
        $('#status').text('connected (' + conn.protocol + ')');
        $('#connect').text('Disconnect');
      }
    }

    function sendJson(msg) {
      // Send JSON object to server.
      console.log("Sending: ");
      console.log(msg);
      sendRaw(JSON.stringify(msg));
    }

    function sendRaw(msg) {
      // Send raw message to server.
      conn.send(msg);
    }

    $('#connect').click(function() {
      if (conn == null) { // If not connected
        connect();
      } else {
        disconnect();
      }

      update_ui();
      return false;
    });

    $('#quit').click(function() {
      sendJson({'type': 'cmd', 'cmd': 'quit'});
      return false;
    });

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

    /*$(".LEDbox").click(function(e){ // When a light button is clicked
        if(e.target.id != "lightsTitle"){
        sendJson({type:"input",data:{input:e.target.id,permissionKey}}); // Sends the target and the key
        }
    });*/

    connect();
    updateDisplay([]);

    //addChat("Hello from another person",{name:"Fake Account",colour:"lime"}); Test chat code
});

var thisSender = {name:"noname",colour:"lime"}; // The default client
var permissionKey = "noKey"; // The default value
var maxCharacters = 220;
var planeTitle = '<b><span style="color:#00d">BLUE-FIREBIRD v1</span> <span style="color:#0ad">Flight --001</span></b>';

//////////////////// DISPLAY ////////////////////

function setupDisplay(){
    var positions = [   ["","","",""], // First ROW
                        ["","",""], // Second ROW
                        ["","",""], // Third ROW
                        ["",""], // Fourth ROW
                    ];

    // Flight logs box
    positions[1][1] = '<div id="logBox" class="controlBox"><span id="logTitle" class="controlTitle">LOGS: '+
                      '<span class="subnote"><span id="numUsers">0</span> user<span id="numUsersPlural">s</span> <span id="numUsersComment">, so sad...</span></span></span><br>'+
                      '<div id="chat"></div>'+
                      '</div>';
    data["numUsers"] = 0;
    data["numUsersComment"] = ", so sad...";
    data["numUsersPlural"] = "s"

    // Video Box
    positions[2][1] = '<div id="videoBox" class="controlBox"><span id="videoTitle" class="controlTitle">LIVE VIDEO FEED </span>'+
                      '<span id="signal" class="subnote noSig">NO SIGNAL</span></span><br>'+
                      '<div id="video"></div>'+
                      '</div>';

    ///// List of data for all boxes /////

    displayData = [{name:"dropdoors",title:"DROP DOORS",row:0,col:1,lines:[
                     {description:"Locked",id:"locked"},
                     {description:"Door Pos",id:"door"},
                   ]},

                   {name:"radio",title:"RADIO LINK",row:0,col:3,lines:[
                     {description:"Onboard RSSI",id:"onboardRSSI"},
                     {description:"Packets Received",id:"radioPacketRate"},
                     {description:"Ground RSSI",id:"groundRSSI"},
                     {description:"Serial Connection",id:"serialConnection"},
                   ]},

                   {name:"livestatus",title:"STATUS",row:0,col:2,lines:[
                     {description:"RasPi Loops",id:"piLoopSpeed"},
                     {description:"NANO Loops",id:"nanoLoopSpeed"},
                     {description:"UNO Loops",id:"unoLoopSpeed"},
                     {description:"Ground Time",id:"groundTime"},
                     {description:"Autopilot Mode",id:"autopilotMode"},
                   ]},

                   {name:"battery",title:"BATTERY",row:1,col:2,lines:[
                     {description:"Voltage",id:"voltage"},
                     {description:"Current Draw",id:"amps"},
                     {description:"Temperature",id:"batteryTemperature"},
                   ]},

                   {name:"other",title:"OTHER DATA",row:2,col:2,lines:[
                     {description:"Altitude",id:"altitude"},
                     {description:"Outside Temp",id:"outsideTemperature"},
                     {description:"Airspeed",id:"airspeed"},
                     {description:"G-Force",id:"gforce"},
                   ]},

                   {name:"ailerons",title:"AILERONS",row:0,col:0,lines:[
                     {description:"Input",id:"aileronsInput"},
                     {description:"Trim Centre",id:"aileronsTrim"},
                     {description:"Control Range",id:"aileronsRange"},
                     {description:"Roll",id:"roll"},
                   ]},

                   {name:"elevator",title:"ELEVATOR",row:1,col:0,lines:[
                     {description:"Input",id:"elevatorInput"},
                     {description:"Trim Centre",id:"elevatorTrim"},
                     {description:"Control Range",id:"elevatorRange"},
                     {description:"Pitch",id:"pitch"},
                   ]},

                   {name:"rudder",title:"RUDDER",row:2,col:0,lines:[
                     {description:"Input",id:"rudderInput"},
                     {description:"Trim Centre",id:"rudderTrim"},
                     {description:"Control Range",id:"rudderRange"},
                     {description:"Yaw",id:"yaw"},
                   ]},

                   {name:"throttle",title:"THROTTLE",row:3,col:0,lines:[
                     {description:"Input",id:"throttleInput"},
                     {description:"Control Range",id:"throttleRange"},
                     {description:"Speed",id:"speed"},
                   ]},
                   ];

    for(var i=0;i<displayData.length;i++){ // Constructs all the controls
        var control = displayData[i];
        var html = "<div id='"+ control.name +"Box' class='controlBox'><span class='controlTitle'>"+ control.title +"</span>";
        for(var j=0;j<control.lines.length;j++){
            var line = control.lines[j];
            html += "<br>";
            html += line.description+": <span class='controlData' id='"+ line.id +"'></span>"; // Adds in the line

            data[line.id] = "--"; // Adds entry into data object
        }
        html += "<div id='"+ control.name +"Img' class='controlImg' style='background-image:url(&quot;static/controls/"+ control.name +".png&quot;)'></div>"; // Adds image
        html += "</div>";

        positions[control.row][control.col] = html;
    }

    ///// TABLE POSITION SETUP /////
    var table = document.getElementById("displayGrid");
    var tableBody = document.createElement("tbody");
    table.appendChild(tableBody);


    for(var x=0;x<positions.length;x++){ // Fills in all cells
        var row = document.createElement("tr");
        tableBody.appendChild(row);
        for(var y=0;y<positions[x].length;y++){
            var cell = document.createElement("td");
            row.appendChild(cell);
            $(cell).html(positions[x][y]);
        }
    }

    ///// ADDITIONAL STYLING /////

    $("#logBox").parent().attr('colspan',2); // Makes the logBox 2 cells wide
    $("#logBox").parent().attr('rowspan',1); // Makes the logBox 1 cell tall
    $("#videoBox").parent().attr('colspan',2);
    $("#videoBox").parent().attr('rowspan',2);

    windowWidth = $(window).width();
    windowHeight = $(window).height();

    $("#displayGrid").width(windowWidth*0.94);
    $("#displayGrid").height(windowHeight*0.8);
    $("#displayGrid").offset({left:windowWidth*0.03,top:windowHeight*(1-0.8-0.03)});

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
}

var data = {};

function updateDisplay(inData){
    console.log(inData);
    for (var k in inData) {
        data[k] = inData[k];
    }
    console.log(data);
    $.each(data, function(key, value) { // Changes the html code
        if(value == "true"){
            $("#"+key).removeClass("off");
            $("#"+key).addClass("on");
        }else if(value == "false"){
            $("#"+key).removeClass("on");
            $("#"+key).addClass("off");
        }else{
            $("#"+key).html(value);
            //console.log("changing "+ key +"'s value to "+ value);
        }
    });
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
            console.log("There was no error-flashing loop to remove from id: "+ id);
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
