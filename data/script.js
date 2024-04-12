// Get current readings when the page loads.

window.addEventListener('load', getReadings);

// Get current date and time function.

function updateDateTime() {

    var currentdate = new Date();

    var datetime = currentdate.getDate() + "/" +
        (currentdate.getMonth() + 1) + "/" +
        currentdate.getFullYear() + " at " +
        currentdate.getHours() + ":" +
        (currentdate.getMinutes() < 10 ? "0" : "") + currentdate.getMinutes() + ":" +
        (currentdate.getSeconds() < 10 ? "0" : "") + currentdate.getSeconds();
    document.getElementById("update-time").innerHTML = datetime;
    console.log(datetime);

} // Close function.

// Function to get current readings on the webpage when it loads for the first time.

function getReadings() {

    var xhr = new XMLHttpRequest();

    xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            var myObj = JSON.parse(this.responseText);
            console.log(myObj);
            for (var i = 0; i < myObj.readings.length; i++) {
                document.getElementById("sessionTitleArray" + i).innerHTML = myObj.readings[i].title;
                document.getElementById("sessionDateArray" + i).innerHTML = myObj.readings[i].date;
                document.getElementById("sessionTimeArray" + i).innerHTML = myObj.readings[i].time;
                document.getElementById("sessionCategory" + i).innerHTML = myObj.readings[i].category;
                document.getElementById("sessionPercentageArray" + i).innerHTML = myObj.readings[i].percentage;
            }
            updateDateTime();
        }
    };

    xhr.open("GET", "/readings", true);
    xhr.send();

} // Close function.

// Create an Event Source to listen for events.

if (!!window.EventSource) {

    var source = new EventSource('/events');

    source.addEventListener('open', function (e) {
        console.log("Events Connected");
    }, false);

    source.addEventListener('error', function (e) {
        if (e.target.readyState != EventSource.OPEN) {
            console.log("Events Disconnected");
        }
    }, false);

    source.addEventListener('new_readings', function (e) {
        console.log("new_readings", e.data);
        var obj = JSON.parse(e.data);
        for (var i = 0; i < obj.readings.length; i++) {
            document.getElementById("sessionTitleArray" + i).innerHTML = obj.readings[i].title;
            document.getElementById("sessionDateArray" + i).innerHTML = obj.readings[i].date;
            document.getElementById("sessionTimeArray" + i).innerHTML = obj.readings[i].time;
            document.getElementById("sessionCategory" + i).innerHTML = obj.readings[i].category;
            document.getElementById("sessionPercentageArray" + i).innerHTML = obj.readings[i].percentage;
        }
        updateDateTime();

    }, false);

} // Close function.
