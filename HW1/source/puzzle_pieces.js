module.exports = class Pieces {
	constructor() {
		// Import the pieces.
		const pieces = require('./puzzle_pieces_data.json');
		this.pieces = [];

		pieces.map((piece) => {
			piece.format.map((side, sideIndex) => {
				side.map((format, isomorphicIndex) => {
					format = {
						shape: format,
						index: isomorphicIndex
					};
					this.pieces.push(new Piece(piece, sideIndex, format));
				});
			});
		});
	}

	getPieces(options) {
		return this.pieces.filter((piece) => {
			if (options && options.color === piece.color) {
				return piece;
			} else if (options === undefined) {
				return piece;
			}
		});
	}
};

class Piece {
	constructor(info, side, format) {
		this.color           = info.color;
		this.abbreviation    = info.abbreviation;
		this.side            = side;
		this.shape           = format.shape;
		this.isomorphicIndex = format.index;
	}
};