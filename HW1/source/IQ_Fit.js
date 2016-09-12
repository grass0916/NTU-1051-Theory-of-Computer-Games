const Pieces = require('./puzzle_pieces.js');

module.exports = class IQ_Fit {
	constructor(rows, cols) {
		// Rows and columns.
		this.rows = rows;
		this.cols = cols;
		// Initial the board.
		this.board = (new Array(rows).fill('row')).map(() => new Array(cols).fill('-'));
		// Import the pieces.
		this.pieces = new Pieces();
	}


	start() {
		console.log(this.pieces.getPieces().length);
		console.log(this.pieces.getPieces({ color: 'blue' }).length);
		console.log(this.pieces.getPieces({ color: 'green' }).length);

		// this.pieces.map((piece) => {
		// 	// console.log(piece.color);
		// 	piece.format.map((group, i) => {
		// 		group.map((shape, j) => {
		// 			// console.log(i, j, shape);

		// 		});
		// 	});
		// 	// console.log("======================\n\n\n");
		// });
	}

	showBoard() {
		return this.board;
	}

	isLegal(position, piece) {
		if (position === undefined) {
			throw (new Error("'position' is not correct."));
		}
		console.log(position);
	}
};