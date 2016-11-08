const fs = require('fs');
const pug = require('pug');
const open = require('open');

const amount = process.argv[2];
let rounds = [];

for (let i = 3; i < process.argv.length; i += 2) {
	const dataFile  = __dirname + '/data/' + process.argv[i];
	const labelName = process.argv[i+1];

	let dataSets = dataNormalize(fs.readFileSync(dataFile, 'utf8'));
	let dataAvg  = averageRemoveOutlier(dataSets);
	rounds.push({ 'name': labelName, 'data': dataAvg });
}

// Write the HTML.
let html = pug.renderFile(__dirname + '/chart.pug', { 'rounds': rounds, 'amount': amount });
fs.writeFile(__dirname + '/chart.html', html, function (err) { });
// Open the browser.
open(__dirname + '/chart.html', function (err) { });

// "1,2,3 4,5,6 7,8,9 will be converted to [[1,2,3],[4,5,6],[7,8,9]]."
function dataNormalize(data) {
	return data.split('\n')
		.filter(function (round) {
			return round !== '';
		}).map(function (round) {
			return round.split(',');
		});
}

function averageRemoveOutlier(data) {
	let _data = data[0].map(function (col, i) { 
		return data.map(function (row) { 
			return row[i] 
		});
	});

	let avg = _data.map(function (values) {
		values = values.sort();
		// Remove the largest and smallest.
		if (values.length >=3) {
			values.shift();
			values.pop();
		}
		// Return the average.
		return values.reduce(function(a, b) { return parseInt(a) + parseInt(b); }) / values.length / 1000000;
	});

	return avg;
}


