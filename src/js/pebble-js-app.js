function locationSuccess(pos) {
  var coordinates = pos.coords;
  console.log("Received coords!");
  console.log("Lat: " + coordinates.latitude);
  Pebble.sendAppMessage({
          '7': coordinates.latitude + '',
          '12': coordinates.longitude + ''
        });
}

function locationError(e) {
  console.log("JS Connection Error");
}

var locationOptions = {
  'timeout': 15000,
  'maximumAge': 60000
};

Pebble.addEventListener('ready', function (e) {
  console.log('connect! ' + e.ready);
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError,
    locationOptions);
  console.log(e.type);
});

Pebble.addEventListener('appmessage', function (e) {
  console.log("received?");
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError,
    locationOptions);
  console.log('message!');
});