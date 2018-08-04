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

var pageOptions = {
    lastN: 1,
    autoUpdate: false
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

    var zoom = d3.zoom()
        .on("zoom", zoomed);

    var svg = d3.select('#linechart')
        .append('svg')
        .attr('width', width + margin.left + margin.right)
        .attr('height', height + margin.top + margin.bottom);

    var chart = svg.append('g')
        .attr('class', 'charts')
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")")
        .call(zoom);

    var formatTime = d3.timeFormat('%m/%d %H:%M');
    
    // original color threshold is 10! after 10 different sensorID change schemeCategory to 20
    var color = d3.scaleOrdinal(d3.schemeCategory10);

    // ----------------------------------------

    var xAxis = d3.axisBottom().scale(xScale).ticks(10).tickFormat(d3.timeFormat('%H:%M'));
    var yAxis = d3.axisLeft().scale(yScale).ticks(10);

    var gXAxis = chart.append('g')
        .attr("class", "x axis")
        .attr("transform", "translate(0," + height + ")")
        .style("dominant-baseline", "central")
        .call(xAxis);
    
    var gYAxis = chart.append('g').attr("class", "y axis").call(yAxis);

    function zoomed() {
        var nextXScale = d3.event.transform.rescaleX(xScale);
        var nextYScale = d3.event.transform.rescaleY(yScale);

        d3.selectAll('.line')
            .style("stroke-width", 2/d3.event.transform.k)
            .attr("transform", d3.event.transform);

        d3.selectAll('.chartdots')
            .style("stroke-width", 2/d3.event.transform.k)
            .attr("transform", d3.event.transform);
        
        gXAxis.call(xAxis.scale(nextXScale));
        gYAxis.call(yAxis.scale(nextYScale));
    }

    // ----------------------------------------
    
    var groupData = data.groupBy('sensorID');
    var dataset = Object.keys(groupData).map(function (key) { return groupData[key]; });
    
    var line = d3.line()
        .x(function(d) { return xScale(mapDate(d)); })
        .y(function(d) { return yScale(mapTemp(d)); });

    var dataBindings = chart.selectAll('g.charts').data(dataset);

    // adding the lines itself
    
    dataBindings.enter()
        .append('g')
        .append('path')
        .attr('class', 'line')
        .attr("fill", "none")
        .attr("stroke", function(d,i) {return color(i);})
        .attr("stroke-width", 2)
        .attr('d', function(d) { return line(d);});

    // ----------------------------------------

    // append zoom area
    var view = chart.append("rect")
        .attr("class", "zoom")
        .attr("width", width)
        .attr("height", height)
        .call(zoom);


    // ----------------------------------------

    // add legend   
    var legend = chart.append("g")
	.attr("class", "legend")
	.attr("height", 400)
	.attr("width", 100)
        .attr('transform', 'translate(-70,50)');

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
        .attr("class", 'tooltip')
        .style("opacity", 0);

        dataBindings.enter()
        .selectAll('circle')
	.data(function (d) { return d; })
        .enter()
        .append('circle')
        .attr('class', 'chartdots')
	.attr('cx', function (d) { return xScale(mapDate(d)); })
	.attr('cy', function (d) { return yScale(mapTemp(d)); })
	.attr('r', 2)
        .on("mouseover", function(d) {
            tooltipDiv.transition().duration(200).style('opacity', 0.9);
            tooltipDiv.html(formatTime(mapDate(d)) + '<br/>'  + d.temperature + ' °C')	
                .style('left', (d3.event.pageX) + 'px')		
                .style('top', (d3.event.pageY - 28) + 'px');	
        })					
        .on("mouseout", function(d) { tooltipDiv.transition().duration(500).style("opacity", 0); });
}

function clearCharts() {
    d3.selectAll("#linechart > *").remove();
}

function updateTime() {
    var updateMark = document.getElementById('lastUpdated');
    var d = new Date();
    updateMark.innerHTML = d.toLocaleTimeString();
}

function loadLastDays(days) {
    pageOptions.lastN = days;
    var url = document.URL + 'temps?days=' + days;

    d3.json(url, function(data) {
        console.log('API response received');
        if (data) {
            clearCharts();
            setupCharts(data);
            updateTime();
        } else {
            console.log('Data is null');
        }
    });
}

function autoUpdate() {
    if (pageOptions.autoUpdate) {
        loadLastDays(pageOptions.lastN);
    }
}

window.onload = function () {
    loadLastDay();
};

function loadLastDay() {
    loadLastDays(1);
}

function loadLast3Days() {
    loadLastDays(3);
}

function autoUpdateClick() {
    var checkBox = document.getElementById("autoUpdateCheckbox");
    if (checkBox.checked) {
        pageOptions.updater = setInterval(autoUpdate, 60*1000);
    } else {
        clearInterval(pageOptions.updater);
    }
}
