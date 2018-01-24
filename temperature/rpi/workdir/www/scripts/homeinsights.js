function mapDate(d) { return new Date(d.time*1000); }
function mapTemp(d) { return d.temperature; }

function setupCharts(data) {
    var margin = {top: 20, right: 20, bottom: 30, left: 50};
    var width = 960 - margin.left - margin.right;
    var height = 500 - margin.top - margin.bottom;
    // set the ranges
    var xScale = d3.scaleTime()
        .range([margin.left, width - margin.right])
        .domain([d3.min(data, mapDate), d3.max(data, mapDate)]);

    var yScale = d3.scaleLinear()
        .range([height - margin.top, margin.bottom])
        .domain([d3.min(data, mapTemp), d3.max(data, mapTemp)]);

    var line = d3.line()
        .x(function(d) { return xScale(mapDate(d)); })
        .y(function(d) { return yScale(mapTemp(d)); });

    var chart = d3.select('#linechart').append('svg')
        .attr('width', width + (2 * margin.left) + margin.right)
        .attr('height', height + margin.top + margin.bottom)
        .append('g').
        attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    chart.append('path')
        .attr("fill", "none")
        .attr("stroke", "steelblue")
        .attr("stroke-width", 1.5)
        .attr('d', line(data));
}

window.onload = function () {
    var url = document.URL + 'temps';
    var list = $('#insights');

    // use only for DEBUG 
    $.getJSON(url, function(data) {
        console.log('API response received');
        $.each(data, function(index, object) {
            var item = $('<li/>');
            item.append('sensor #' + object.sensorID + ': ' + object.temperature + 'C');
            list.append(item);
        });

        setupCharts(data);
    });
};



