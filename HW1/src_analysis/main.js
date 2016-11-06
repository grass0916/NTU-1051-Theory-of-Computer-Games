const DFS_OUTPUT = __dirname + '/../debug_5_5/elapsed_DFS_(5x5)_q1000_t10.txt';
const BFS_OUTPUT = __dirname + '/../debug_5_5/elapsed_BFS_(5x5)_q1000_t10.txt';

const fs = require('fs');
const pug = require('pug');

let roundsDFS = dataNormalize(fs.readFileSync(DFS_OUTPUT, 'utf8'));
let roundsBFS = dataNormalize(fs.readFileSync(BFS_OUTPUT, 'utf8'));

let avgDFS = averageRemoveOutlier(roundsDFS);
let avgBFS = averageRemoveOutlier(roundsBFS);

let rounds = [avgDFS, avgBFS];

let html = pug.renderFile('chart.pug', { 'rounds': rounds, 'amount': 1000 });

fs.writeFile('./chart.html', html, function (err) { });


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
		values.shift();
		values.pop();
		// Return the average.
		return values.reduce(function(a, b) { return parseInt(a) + parseInt(b); }) / values.length;
	});

	return avg;
}


