let ReconnectingWebSocket = require('reconnectingwebsocket');

var currentTemperature = 0;
var startReflowTime = Date.now();
var xs = [];
var temps = [];
var setpoints = [];


connection = new ReconnectingWebSocket(wsUri, ['arduino']);
connection.reconnectInterval = 2000;

connection.onopen = function () {
  connection.send('Connect ' + new Date());
};
connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {
  handleRequest(e.data);
};

function handleRequest(request) {
  let vals = request.split(':');
  switch (vals[0]) {
    case 't':
      document.getElementById('sensTemp').innerHTML = vals[1];
      currentTemperature = vals[1];
      document.getElementById('setpoint').innerHTML = vals[2];
      document.getElementById('pidKp').innerHTML = vals[3];
      document.getElementById('pidKi').innerHTML = vals[4];
      document.getElementById('pidKd').innerHTML = vals[5];

      xs.push(~~((Date.now()-startReflowTime)/1000));
      temps.push(parseFloat(vals[1]));
      setpoints.push(parseFloat(vals[2]));
      myChart.update('none');

      /*console.log('temp: ' + parseFloat(vals[1]) + ' setpoint: ' + parseFloat(vals[2]));*/
      break;
    default:
      console.log('handleRequest function: Default');
  }
};


function startPreheat() {
  console.log('Sent start preheat trigger');
  connection.send('1');
  startReflowTime = Date.now();
};
document.getElementById('startPreheat').addEventListener('click', startPreheat, false);

function startReflow() {
  console.log('Sent start reflow trigger');
  connection.send('2');
  startReflowTime = Date.now();
};
document.getElementById('startReflow').addEventListener('click', startReflow, false);

function stopReflow() {
  console.log('Sent stop reflow trigger');
  connection.send('3');
};
document.getElementById('stopReflow').addEventListener('click', stopReflow, false);



const ctx = document.getElementById('reflowPlot').getContext('2d');
var myChart = new Chart(ctx, {
  type: 'line',
  data: {
    labels: xs,
    datasets: [
      {
        label: 'is',
        data: temps,
        fill: false,
        backgroundColor: 'rgba(255, 99, 132, 0.2)',
        borderColor: 'rgba(255, 159, 64, 1)',
        borderWidth: 1,
      },
      {
        label: 'setpoint',
        data: setpoints,
        fill: true,
        backgroundColor: 'rgba(255, 132, 99, 0.2)',
        borderColor: 'rgb(54, 162, 235, 1)',
        borderWidth: 1,
      }
    ]
  },
  options: {
    animation: false,
    responsive: true,
    maintainAspectRatio: false,
    scales: {
      x: {
        beginAtZero: true,
      },
      y: {
        beginAtZero: true,
        max: 280,
        title: {
          display: true,
          text: 'value'
        }
      }
    },
    plugins: {
      title: {
        display: true,
        text: 'Temperature plot'
      }
    }
  }
});