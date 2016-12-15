#include "board.h"
#include <random>
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <sstream>

constexpr char m_tolower(char c){
    return c + ('A' <= c && c <= 'Z') * ('a' - 'A');
}

constexpr unsigned my_hash(const char *s, unsigned long long int hv = 0) {
    return *s && *s != ' ' ? my_hash(s + 1, (hv * ('a' + 1) + m_tolower(*s)) % 0X3FFFFFFFU) : hv;
}

struct history {
    int x, y, pass, tiles_to_flip[27], *ed;
};

template<class RIT>RIT random_choice(RIT st, RIT ed){
	static std::mt19937 local_rand(std::random_device {} ());
	return st+std::uniform_int_distribution<int> (0, ed-st-1) (local_rand);
}

// Ranomly OTP.
class ROTP {
public:
	ROTP() : B(), HED(H) {
		do_init();
	}

	// Used into simulations.
	bool do_op(const char *cmd) {
		switch(my_hash(cmd)) {
			case my_hash("play"): {
				int x, y;
				sscanf(cmd, "%*s %d %d", &x, &y);
				do_play(x, y, "cmd_play_1");
				return true;
			}

			case my_hash("genmove"): {
				int xy = do_genmove();
				int x = xy/8, y = xy%8;
				do_play(x, y, "cmd_genmove_1");
				return true;
			}

			case my_hash("quit"):
				return false;

			default:
				return true;
		}
	}

	bool do_op(const char *cmd, char *out, FILE *myerr) {
		switch (my_hash(cmd)) {
			case my_hash("name"):
				sprintf(out, "name M10415096");
				return true;

			case my_hash("clear_board"):
				do_init();
				B.show_board(myerr);
				sprintf(out, "clear_board");
				return true;

			case my_hash("showboard"):
				B.show_board(myerr);
				sprintf(out, "showboard");
				return true;

			case my_hash("play"): {
				int x, y;
				sscanf(cmd, "%*s %d %d", &x, &y);
				do_play(x, y, "cmd_play_2");
				B.show_board(myerr);
				sprintf(out, "play");
				return true;
			}

			case my_hash("genmove"): {
				int xy = do_genmove();
				int x = xy/8, y = xy%8;
				do_play(x, y, "cmd_genmove_2");
				B.show_board(myerr);
				sprintf(out, "genmove %d %d", x, y);
				return true;
			}

			case my_hash("quit"):
				sprintf(out, "quit");
				return false;

			default:
				sprintf(out, "unknown command");
				return true;
		}
	}

	std::string get_html(unsigned, unsigned) const;

	bool isGameOver() {
		return this->B.is_game_over();
	}

	int getFinalScore() {
		return this->B.get_score();
	}

	int* getValidMove(int* oit) {
		return this->B.get_valid_move(oit);
	}

protected:
	board B;
	history H[128], *HED;

	// Initialize in do_init.
	void do_init() {
		B = board();
		HED = H;
		// Initial the History.
		for (int i = 0; i < 128; i++) { H[i].x = -1, H[i].y = -1, H[i].pass = 0; }
	}

	// Randomly select a move in do_genmove.
	virtual int do_genmove() {
		int ML[64], *MLED(B.get_valid_move(ML));
		return MLED == ML ? 64 : *random_choice(ML, MLED);
	}

	// Update board and history in do_play
	void do_play(int x, int y, std::string caller) {
		if (HED != std::end(H) && B.is_game_over() == 0 && B.is_valid_move(x, y)) {
			HED->x = x, HED->y = y;
			HED->pass = B.get_pass();
			HED->ed   = B.update(x, y, HED->tiles_to_flip);
			++HED;
		} else {
			std::stringstream ss;
			ss << "Wrong play from '" << caller << "' on (" << x << ", " << y << ").\n";
			fputs(ss.str().c_str(), stderr);
		}
	}
};

/*
	This class inherits from the native OTP, 
	but using Monte-Carlo tree search to do 'genmove' action.

	Salmon written here.
*/

// #define DEBUG true

#define VISITS_LIMIT 100000
#define UCB_CONSTANT 1.414
#define PRUNE_ITERS 1000

#define XY2X(xy) (xy / 8)
#define XY2Y(xy) (xy % 8)

#define Branches std::vector<Visitation*>

// Play chess on the sepcific (x, y) of node.
#define PLAY(n) \
	do { \
		std::stringstream command; \
		command << "play " << XY2X(n->nextXY) << " " << XY2Y(n->nextXY); \
		game.do_op(command.str().c_str()); \
	} while (0)

// UCB fomula: Wi / Ni + c * sqrt(log(N) / Ni)
#define UCB(vis, i) ((! vis->visits) ? INFINITY : ((! vis->isAvailable) ? -INFINITY : vis->wins / vis->visits + UCB_CONSTANT * std::sqrt(std::log(i+1) / vis->visits)))
// Domain knowledge for getting weight with specific position.
#define KNOWLEDGE(vis) ((vis->nextXY == 0 || vis->nextXY == 7 || vis->nextXY == 56 || vis->nextXY == 63) ? 2 : 1)

struct Visitation {
	// Is still available due to different round.
	bool isAvailable;
	// Next selected position.
	int nextXY;
	// Times of visiting and winning.
	double visits, wins;
	// Parent node.
	Visitation *parent;
	// Is still can be expand the children nodes.
	bool isExpandable;
	// Children nodes.
	Branches branches;
	// Quickly reference score. Like UCB score.
	double score;
};

class OTP : public ROTP {
public:
	// The root of Monte Carlo Tree Search.
	Visitation *root;

	// Constructor.
	OTP() : ROTP() {
		this->root = new Visitation;
		// Inintial for root.
		this->root->isAvailable  = true;
		this->root->isExpandable = true;
		this->root->nextXY = -1;
		this->root->visits = 0;
		this->root->wins   = 0;
		this->root->score  = 0;
		this->root->parent = NULL;
	}

private:
	// Choose the best move in do_genmove.
	int do_genmove() override {
		// If tile is 'X': 1, 'O': 2. Then transfers to 1 or 0.
		int myTile = B.get_my_tile() == 1 ? 1 : 0;
		// Virtual root for this iteration.
		Visitation *rt = this->root;

		for (int i = 0; i < VISITS_LIMIT; i++) {
			// Simulate a board.
			ROTP game;
			// Selected node.
			Visitation *sn = rt = this->root;

			// ======== [Preprocess] ======== //
			// Import the history of current state.
#ifdef DEBUG
			std::cout << "\n\tHistory: \n";
#endif
			for (int h = 0; H[h].x > -1; h++) {
				// Play action for redoing history.
				std::stringstream command;
				command << "play " << this->H[h].x << " " << this->H[h].y;
				game.do_op(command.str().c_str());
				// Update the root.
				int wannaFind = H[h].x * 8 + H[h].y;
				auto findIter = std::find_if(
					sn->branches.begin(), sn->branches.end(),
					[wannaFind] (Visitation const *vis) {
						return vis->nextXY == wannaFind;
					}
				);
				// If found the node, change the root for this iteration.
				if (findIter != sn->branches.end()) {
					// Update the root.
					sn = rt = *findIter;
				}
				// If not found the node, create a new one and make the root point that.
				else {
					Visitation *vis = new Visitation;
					vis->isAvailable = vis->isExpandable = true;
					vis->nextXY = wannaFind;
					vis->visits = vis->wins = vis->score = 0;
					vis->parent = sn;
					// Insert into the branches.
					sn->branches.push_back(vis);
					// Update the root.
					sn = rt = vis;
				}
#ifdef DEBUG
				std::cout << "\t\t" << (h+1) << ": " << command.str().c_str() << "\n";
#endif
			}

			// Progress Pruning.
			if (i % PRUNE_ITERS == 0 && i > 0) {
				int threshold = i / sn->branches.size();
				for (Visitation *vis : sn->branches) {
					if (vis->visits < threshold) {
						vis->isAvailable = false;
					}
				}
			}

			// ======== [Selection] ======== //
			// Calculate all UCB score, then choose the maximum.
#ifdef DEBUG
			std::cout << "\tWalked on: ";
#endif
			for (int rd = 1; sn->isExpandable == false && sn->branches.size() > 0; rd++) {
				// The the biggest UCB score from current depth.
				auto maxIter = (rd % 2 == myTile) ?
					std::max_element(
						sn->branches.begin(), sn->branches.end(),
						[i] (Visitation const *vis1, Visitation const *vis2) {
							// Add the domain knowledge, the weights for different XY.
							return UCB(vis1, i) * KNOWLEDGE(vis1) < UCB(vis2, i) * KNOWLEDGE(vis2);
						}
					) : std::min_element(
						sn->branches.begin(), sn->branches.end(),
						[i] (Visitation const *vis1, Visitation const *vis2) {
							return UCB(vis1, i) * KNOWLEDGE(vis1) > UCB(vis2, i) * KNOWLEDGE(vis2);
						}
					)
				;
				// Update the selected node.
				sn = *maxIter;
				// Play the selected (x, y) on the simulated game.
				PLAY(sn);
#ifdef DEBUG
				std::cout << "->" << sn->nextXY;
#endif
			}
#ifdef DEBUG
			std::cout << "\n";
#endif

			// ======== [Expansion] ======== //
			// Get the all possibile positions.
        	int ML[64], *MLED(game.getValidMove(ML));
			// Expand the branches.
			int *ptr = ML + sn->branches.size();
			// Create a new visitation.
			if (ptr != MLED) {
				Visitation *vis = new Visitation;
				vis->isAvailable = vis->isExpandable = true;
				vis->nextXY = (*ptr);
				vis->visits = vis->wins = vis->score = 0;
				vis->parent = sn;
				// Insert into the branches.
				sn->branches.push_back(vis);
#ifdef DEBUG
				std::cout << "\tExpand on " << sn->nextXY << " is '" << vis->nextXY << "'.\n";
#endif
				// Close this node to expand.
				if (ptr+1 == MLED) sn->isExpandable = false;
				// Update the selected node.
				sn = sn->branches.back();
				// Play the selected (x, y) on the simulated game.
				PLAY(sn);
			}


			// ======== [Simulation] ======== //
			// Use the native 'genmove' function to simulate.
			for (int t = 0; t < 128; t++) {
				// If the game is end, stop the simulation.
				if (game.isGameOver()) { break; }
				// Execute the command for 'genmove'.
				game.do_op("genmove");
			}


			// ======== [Propagation] ======== //
			// Get the final score of game, then record into node.
			int score = (myTile ? 1 : -1) * game.getFinalScore();
			// Visits + 1; Win: 1, tie: 0.5, lose: 0.
			double winsWeight = score > 0 ? 1 : (score < 0 ? 0 : 0.5);
			for (Visitation *visPtr = sn; visPtr != NULL; visPtr = visPtr->parent) {
				visPtr->visits += 1;
				visPtr->wins   += winsWeight;
			}
#ifdef DEBUG
			std::cout << "\tResult: " << sn->wins << " wins in " << sn->nextXY << ".\n";
			std::cout << "\n==== End " << i+1 << " simulation ====\n";
#endif
		}

		// Choose the best winning rate.
		auto maxIter = std::max_element(
			rt->branches.begin(), rt->branches.end(),
			[] (Visitation const *vis1, Visitation const *vis2) {
				return vis1->wins / vis1->visits < vis2->wins / vis2->visits;
			}
		);
		int bestXY = (rt->branches.begin() == rt->branches.end() ? 64 : (*maxIter)->nextXY);

		// Messages about simulations.
		std::cout << "\n=== Multi-simulations in genmove ===\n";
		for (Visitation *vis : rt->branches) {
			std::cout << "    " << vis->nextXY << ": (" << vis->wins << "/" << vis->visits << ")\n";
		}
		std::cout << "\n\tSelect: " << bestXY << "(" << XY2X(bestXY) << "," << XY2Y(bestXY) << ")\n";
		std::cout << "====================================\n\n\n";

		return bestXY;
	}
};
