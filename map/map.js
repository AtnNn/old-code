// http://code.google.com/intl/en/apis/maps/documentation/javascript/reference.html

var bounds;
var map;
var latlngs;
var geocoder;

function initMap(id, markers){
  var latlng = new google.maps.LatLng(-34.397, 150.644);
  var myOptions = {
    zoom: 8,
    center: latlng,
    mapTypeId: google.maps.MapTypeId.ROADMAP
  };

  map = new google.maps.Map(document.getElementById(id), myOptions);

  bounds = new google.maps.LatLngBounds();
  latlngs = [];

  geocoder = new google.maps.Geocoder();

  var to = 0;

  $.each(markers, function(){
    var self = this;
    var f = function(){lookup(self);};
    setTimeout(f, to);
    to = to + 1000;
  });
}

function lookup(marker){
          var text = marker.text;
           console.log("looking up: " + text);
          geocoder.geocode({
                             "address": marker.position
                           },
                           function(result, status){
                             if(status == "OK"){
                               var latlng = result[0].geometry.location;
                               console.log(text + ": " + latlng);
                               bounds.extend(latlng);
                               map.fitBounds(bounds);
                               addMarker(map, marker, latlng);
                             }else{
                               console.log(text + ": " + status);
                             }
                           });
        }

function addMarker(map, marker, latlng){
  console.log("Marker", arguments)
  new google.maps.Marker({
               "animation": google.maps.Animation.DROP,
               "map": map,
               "position": latlng,
               "icon": "point.gif"
             });
}
