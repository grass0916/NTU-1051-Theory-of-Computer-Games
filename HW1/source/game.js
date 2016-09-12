// This is a puzzle game "IQ Fit" in Node.js version.

/*
 ____        _                        _            
/ ___|  __ _| |_ __ ___   ___  _ __  | |___      __
\___ \ / _` | | '_ ` _ \ / _ \| '_ \ | __\ \ /\ / /
 ___) | (_| | | | | | | | (_) | | | || |_ \ V  V / 
|____/ \__,_|_|_| |_| |_|\___/|_| |_(_)__| \_/\_/  

	Author: Ze-Hao, Wang (Salmon)
	GitHub: http://github.com/grass0916
	Site:   http://salmon.tw

	Copyright 2016 Salmon
	Released under the GPLv3 license

	[!] SmartGames has whole copyright of "IQ Fit", please don't use the project for commercial.

*/

const ROWS = 5;
const COLS = 10;

const IQ_Fit = require('./IQ_Fit.js');

try {
	let game = new IQ_Fit(ROWS, COLS);
	game.showBoard();
} catch (e) {
	console.log(e);
}