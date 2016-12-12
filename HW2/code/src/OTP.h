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
    ROTP():B(), HED(H) {
        do_init();
    }

    bool do_op(const char *cmd, char *out) {
        switch(my_hash(cmd)) {
            case my_hash("play"):{
                int x, y;
                sscanf(cmd, "%*s %d %d", &x, &y);
                do_play(x, y);
                sprintf(out, "play");
                return true;
            }

            case my_hash("genmove"):{
                int xy = do_genmove();
                int x = xy/8, y = xy%8;
                do_play(x, y);
                sprintf(out, "genmove %d %d", x, y);
                return true;
            }

            case my_hash("final_score"):
                sprintf(out, "final_score %d", B.get_score());
                return true;

            case my_hash("quit"):
                sprintf(out, "quit");
                return false;

            default:
                sprintf(out, "unknown command");
                return true;
        }
    }

    bool do_op(const char *cmd, char *out, FILE *myerr) {
        switch(my_hash(cmd)) {
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

            case my_hash("play"):{
                int x, y;
                sscanf(cmd, "%*s %d %d", &x, &y);
                do_play(x, y);
                B.show_board(myerr);
                sprintf(out, "play");
                return true;
            }

            case my_hash("genmove"):{
                int xy = do_genmove();
                int x = xy/8, y = xy%8;
                do_play(x, y);
                B.show_board(myerr);
                sprintf(out, "genmove %d %d", x, y);
                return true;
            }

            case my_hash("undo"):
                do_undo();
                sprintf(out, "undo");
                return true;

            case my_hash("final_score"):
                sprintf(out, "final_score %d", B.get_score());
                return true;

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

protected:
    board B;
    history H[128], *HED;

    // Initialize in do_init.
    void do_init() {
        B = board();
        HED = H;
		// Initial the H.
		for (int i = 0; i < 128; i++) { H[i].x = -1, H[i].y = -1, H[i].pass = 0; }
    }

    /*
		Randomly select a move in do_genmove.

		Salmon written here.
	*/
    virtual int do_genmove() {
        int ML[64], *MLED(B.get_valid_move(ML));
        return MLED == ML ? 64 : *random_choice(ML, MLED);
    }

    // Update board and history in do_play
    void do_play(int x, int y) {
        if (HED != std::end(H) && B.is_game_over() == 0 && B.is_valid_move(x, y)) {
            HED->x = x;
            HED->y = y;
            HED->pass = B.get_pass();
            HED->ed = B.update(x, y, HED->tiles_to_flip);
            ++HED;
        } else {
            fputs("wrong play.\n", stderr);
        }
    }

    // Undo board and history in do_undo
    void do_undo(){
        if (HED != H) {
            --HED;
            B.undo(HED->x, HED->y, HED->pass, HED->tiles_to_flip, HED->ed);
        } else {
            fputs("wrong undo.\n", stderr);
        }
    }
};

/*
	This class inherits from the native OTP, 
	but using Monte-Carlo tree search to do 'genmove' action.

	Salmon written here.
*/

struct Visitation {
	// Next selected position.
	int nextXY;
	// Times of visiting and winning.
	int visits;
	double wins;
};

class OTP : public ROTP {
private:
    // Choose the best move in do_genmove.
    int do_genmove() override {
        int ML[64], *MLED(B.get_valid_move(ML));

		// If tile is 'X': 1; tile is 'O': 2. Then transfer to 1 and -1.
		int myTile = B.get_my_tile() == 1 ? 1 : -1;

		// Slected position, 0 ~ 63 and pass(64).
		int selXY;

		// Get the all possibile positions.
		std::vector<Visitation> visitations;
		for (int *ptr = ML; ptr != MLED; ptr++) {
			int posXY = *ptr;
			// Create a new visitation.
			Visitation vis;
			vis.nextXY = posXY;
			vis.visits = 0;
			vis.wins   = 0;
			visitations.push_back(vis);
			// std::cout << posXY/8 << "," << posXY%8 << "   ";
		}
		// std::cout << "times: " << visitations.size() << "\n";

		int VISITS_LIMIT = 200;

		for (int i = 0; i < VISITS_LIMIT; i++) {
			// Simulate a board.
			ROTP game;

			// Import the history of current state.
			char obuf[1024];
			for (int i = 0; H[i].x > -1; i++) {
				std::stringstream command;
				command << "play " << this->H[i].x << " " << this->H[i].y;
				// std::cout << "\t" << (i+1) << ": " << command.str().c_str() << "\n";
				game.do_op(command.str().c_str(), obuf);
			}

			// [Selection] Using the UCB value.
			// Calculate all UCB values.
			int maxVisitXY = 64;
			double maxUCB = 0;
			for (Visitation vis : visitations) {
				double valUCB = (vis.visits == 0) ? INFINITY : (double) vis.wins / vis.visits + 1.414 * std::log(VISITS_LIMIT) / vis.visits;
				if (valUCB >= maxUCB) { maxVisitXY = vis.nextXY, maxUCB = valUCB; }
			}

			selXY = maxVisitXY;

			// [Expansion] Import the selected position.
			std::stringstream command;
			command << "play " << selXY/8 << " " << selXY%8;
			game.do_op(command.str().c_str(), obuf);

			// [Simulation].
			for (int i = 0; i < 128; i++) {
				if (game.isGameOver()) { break; }
				game.do_op("genmove", obuf);
			}

			// [Propagation].
			game.do_op("final_score", obuf);
			int score = myTile * std::stoi(std::string(obuf).erase(0, 12));
			for (Visitation &vis : visitations) {
				if (vis.nextXY == selXY) {
					vis.visits += 1;
					vis.wins   += (score > 0 ? 1 : (score < 0 ? 0 : 0.5));
					break;
				}
			}
		}

		for (Visitation vis : visitations) {
			std::cout << vis.nextXY << ": (" << vis.wins << "/" << vis.visits << ")\n";
		}

		return selXY;
	}
};
