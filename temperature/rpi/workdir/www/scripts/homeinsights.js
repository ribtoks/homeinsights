window.onload = function () {
    var url = document.URL + 'temps/';
    var list = $('#list')
    
    $.getJSON(url, function(data) {
        console.log('API response received');
        $(data).each(function(index, object) {
            var item = $('<li/>')
            item.append('sensor #' + object.sensorID + ': ' + object.temperature + 'C');
            list.append(item);
        });
    });
};
