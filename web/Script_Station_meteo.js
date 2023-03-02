
/*let pahoConfig = {
    hostname: "192.168.1.6",  //The hostname is the url, under which your FROST-Server resides.
    port: "1883",           //The port number is the WebSocket-Port,
                            // not (!) the MQTT-Port. This is a Paho characteristic.
    clientId: "ClientId"    //Should be unique for every of your client connections.
}

client = new Paho.MQTT.Client(pahoConfig.hostname, Number(pahoConfig.port), pahoConfig.clientId);
client.onConnectionLost = onConnectionLost;
client.onMessageArrived = onMessageArrived;

client.connect({
onSuccess: onConnect
});

function onConnect() {
// Once a connection has been made, make a subscription and send a message.
console.log("Connected with Server");
client.subscribe("v1.0/Observations");
}

function onConnectionLost(responseObject) {
if (responseObject.errorCode !== 0) {
    console.log("onConnectionLost:" + responseObject.errorMessage);
}
}
function onMessageArrived(message) {
console.log("onMessageArrived:" + message.payloadString);
let j = JSON.parse(message.payloadString);
handleMessage(j);
}*/

const barCanvas = document.getElementById
("barCanvas");

const barChart = new Chart(barCanvas,{
    type:"bar",
    data:{
        labels:["Beijing","Tokyo","Seoul","Hong Kong"],
        datasets:[{}]
    }
})