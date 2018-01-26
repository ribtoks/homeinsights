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
    var margin = {top: 20, right: 60, bottom: 30, left: 50};

    var chartFullHeight = 500;
    var width = 960 - margin.left - margin.right;
    var height = chartFullHeight - margin.top - margin.bottom;

    // set the ranges
    var xScale = d3.scaleTime()
        .range([margin.left, width - margin.right])
        .domain([d3.min(data, mapDate), d3.max(data, mapDate)]);

    var yScale = d3.scaleLinear()
        .range([height - margin.top, margin.bottom])
        .domain([d3.min(data, mapTemp), d3.max(data, mapTemp)]);

    var chart = d3.select('#linechart').append('svg')
        .attr('width', width + margin.left + margin.right)
        .attr('height', height + margin.top + margin.bottom)
        .append('g').
        attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    var formatTime = d3.timeFormat('%m/%d %H:%M');
    
    // original color threshold is 10! after 10 different sensorID change schemeCategory to 20
    var color = d3.scaleOrdinal(d3.schemeCategory10);

    // ----------------------------------------

    var xAxis = d3.axisBottom().scale(xScale).ticks(10).tickFormat(d3.timeFormat('%H:%M'));
    var yAxis = d3.axisLeft().scale(yScale).ticks(10);

    chart.append('g')
        .attr("class", "x axis")
        .attr("transform", "translate(0," + height + ")")
        .style("dominant-baseline", "central")
        .call(xAxis);
    
    chart.append('g').attr("class", "y axis").call(yAxis);

    // ----------------------------------------
    
    var groupData = data.groupBy('sensorID');
    var dataset = Object.keys(groupData).map(function (key) { return [groupData[key]]; });
    
    var line = d3.line()
        .x(function(d) { return xScale(mapDate(d)); })
        .y(function(d) { return yScale(mapTemp(d)); });

    var dataBindings = chart.selectAll('g.line').data(dataset);

    // adding the lines itself
    
    dataBindings.enter()
        .append('path')
        .attr("fill", "none")
        .attr("stroke", function(d,i) {return color(i);})
        .attr("stroke-width", 2)
        .attr('d', function(d) { return line(d);});

    // ----------------------------------------

    // add legend   
    var legend = chart.append("g")
	.attr("class", "legend")
	.attr("height", 400)
	.attr("width", 100)
        .attr('transform', 'translate(-50,50)');

    var legends = { paddingLeft: 25, itemHeight: 25, textPadding: 20 };
    
    legend.selectAll('rect')
        .data(dataset)
        .enter()
        .append("rect")
	.attr("x", width + legends.paddingLeft)
        .attr("y", function(d, i){ return i * legends.itemHeight;})
	.attr("width", 10)
	.attr("height", 10)
	.style("fill", function(d, i) { return color(i); });
      
    legend.selectAll('text')
        .data(dataset)
        .enter()
        .append("text")
	.attr("x", width + legends.paddingLeft + legends.textPadding)
        .attr("y", function(d, i){ return i * legends.itemHeight + 11;})
	.text(function(d) { return 'SensorID: ' + d[0].sensorID; });

    // ----------------------------------------

    var tooltipDiv = d3.select('body').append('div')	
        .attr("class", "tooltip")				
        .style("opacity", 0);

        dataBindings.enter()
        .selectAll('circle')
	.data(function (d) { return d; })
        .enter()
        .append('circle')
	.attr('cx', function (d) { return xScale(mapDate(d)); })
	.attr('cy', function (d) { return yScale(mapTemp(d)); })
	.attr('r', 3)
        .on("mouseover", function(d) {
            tooltipDiv.transition().duration(200).style('opacity', .9);		
            tooltipDiv.html(formatTime(d.time) + '<br/>'  + d.temperature + ' °C')	
                .style('left', (d3.event.pageX) + 'px')		
                .style('top', (d3.event.pageY - 28) + 'px');	
        })					
        .on("mouseout", function(d) { tooltipDiv.transition().duration(500).style("opacity", 0); });
}

window.onload = function () {
    var url = document.URL + 'temps';

    d3.json(url, function(data) {
        console.log('API response received');
        if (data) {
            setupCharts(data);
        } else {
            console.log('Data is null');
        }
    });
};



