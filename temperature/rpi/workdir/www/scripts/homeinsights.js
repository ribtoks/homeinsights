function mapDate(d) { return new Date(d.time*1000); }
function mapTemp(d) { return d.temperature; }

Array.prototype.groupBy = function(prop) {
    return this.reduce(function(groups, item) {
        var val = item[prop];
        groups[val] = groups[val] || [];
        groups[val].push(item);
        return groups;
    }, {});
};

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

    var chart = d3.select('#linechart').append('svg')
        .attr('width', width + (2 * margin.left) + margin.right)
        .attr('height', height + margin.top + margin.bottom)
        .append('g').
        attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    var formatTime = d3.timeFormat('%m/%d %H:%M');
    var div = d3.select('body').append('div')	
        .attr("class", "tooltip")				
        .style("opacity", 0);

    
    var xAxis = d3.axisBottom().scale(xScale).ticks(10).tickFormat(d3.timeFormat('%H:%M'));
    var yAxis = d3.axisLeft().scale(yScale).ticks(10);

    chart.append('g')
	.attr("class", "x axis")
	.attr("transform", "translate(0," + height + ")")
	.call(xAxis);

    chart.append('g')
      .attr("class", "y axis")
      .call(yAxis);
    
    //original color threshold is 10! after 10 different sensorID change schemeCategory to 20
    var color = d3.scaleOrdinal(d3.schemeCategory10);
    
    var groupData = data.groupBy('sensorID');
    $.each(groupData, function(key, values) {
        console.log('sensorID: ' + key);
        
        var line = d3.line()
            .x(function(d) { return xScale(mapDate(d)); })
            .y(function(d) { return yScale(mapTemp(d)); });
        
        chart.append('path')
            .attr("fill", "none")
            .attr("stroke", color(key))
            .attr("stroke-width", 2)
            .attr('d', line(values));

    });

    chart.selectAll("dot")
        .data(data)
        .enter().append("circle")
        .attr("r", 3.5)
        .attr("cx", function(d) { return xScale(mapDate(d)); })
        .attr("cy", function(d) { return yScale(mapTemp(d)); })
        .on("mouseover", function(d) {		
            div.transition()		
                .duration(200)		
                .style('opacity', .9);		
            div	.html(formatTime(d.time) + '<br/>'  + d.temperature + ' °C')	
                .style('left', (d3.event.pageX) + 'px')		
                .style('top', (d3.event.pageY - 28) + 'px');	
        })					
        .on("mouseout", function(d) {		
            div.transition()		
                .duration(500)		
                .style("opacity", 0);	
        });

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



