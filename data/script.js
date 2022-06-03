window.addEventListener('load', getReadings);

Highcharts.setOptions({
    time: {
        timezoneOffset: 36000 //Add your time zone offset here in seconds
    }
});

var chartV = new Highcharts.Chart({
    chart: {
        renderTo: 'chart-voltage'
    },
    series: [
        {
            name: 'INA219'
        }
    ],
    title: {
        text: undefined
    },
    plotOptions: {
        line: {
            animation: false,
            dataLabels: {
                enabled: true
            }
        }
    },
    xAxis: {
        type: 'datetime',
        dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
        title: {
            text: 'Voltage'
        }
    },
    credits: {
        enabled: true
    }
});

var chartI = new Highcharts.Chart({
    chart: {
        renderTo: 'chart-current'
    },
    series: [{
        name: 'INA219'
    }],
    title: {
        text: undefined
    },
    plotOptions: {
        line: {
            animation: false,
            dataLabels: {
                enabled: true
            }
        },
        series: {
            color: '#50b8b4'
        }
    },
    xAxis: {
        type: 'datetime',
        dateTimeLabelFormats: { second: '%H:%M:%S' }
    },
    yAxis: {
        title: {
            text: 'Current (mA)'
        }
    },
    credits: {
        enabled: false
    }
});

function plotVoltage(timeValue,value) {
    var x = (new Date(timeValue*1000)).getTime()
    console.log(x);
    var y = Number(value);
    console.log(value);
    if (chartV.series[0].data.length > 40) {
        chartV.series[0].addPoint([x, y], true, true, true);
    } else {
        chartV.series[0].addPoint([x, y], true, false, true);
    }
}

function plotCurrent(timeValue,value) {
    console.log(timeValue);
    var x = (new Date(timeValue*1000)).getTime();
    console.log(x);
    var y = Number(value);
    if (chartI.series[0].data.length > 40) {
        chartI.series[0].addPoint([x, y], true, true, true);
    } else {
        chartI.series[0].addPoint([x, y], true, false, true);
    }
}

function getReadings() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            var myObj = JSON.parse("["+this.responseText.slice(0,-1) + "]");
            var len = myObj.length;
            // console.log(myObj);
            if (len > 40) {
                for(var i = len -40; i<len; i++){
                    plotVoltage(myObj[i].time, myObj[i].voltage);
                    plotCurrent(myObj[i].time, myObj[i].current);
                }
            } else {
                for(var i = 0; i<len; i++){
                    plotVoltage(myObj[i].time, myObj[i].voltage);
                    plotCurrent(myObj[i].time, myObj[i].current);
                }
            }
            // var volt = myObj.voltage;
            // var amps = myObj.current;
            
            // plotVoltage(volt);
            // plotCurrent(amps);
        }
    };
    xhr.open("GET", "/readings", true);
    xhr.send();
}



if (!!window.EventSource) {
    var source = new EventSource('/events');

    source.addEventListener('open', function (e) {
        console.log("Events connected");
    }, false);

    source.addEventListener('error', function (e) {
        if (e.target.readyState != EventSource.OPEN) {
            console.log("Events Disconnected");
        }
    }, false);

    source.addEventListener('message', function (e) {
        // console.log("message", e.data);
    }, false);

    source.addEventListener('new_readings', function (e) {
        // console.log("new_readings", e.data);
        var myObj = JSON.parse(e.data);
        console.log(myObj);
        plotVoltage(myObj.time, myObj.voltage);
        plotCurrent(myObj.time, myObj.current);
        // document.getElementById("ResultsFile").innerHTML += myObj.time + ", " + myObj.voltage + ", " + myObj.current;
        // document.getElementById("ResultsFile").innerHTML += "<br/>";
    }, false);

    
}