# Move to the project root.
BASEDIR=$(dirname "$0")
cd $BASEDIR

# Create the debug folder.
rm -rf ./debug
mkdir -p ./debug

# Compiler the program.
g++ ./src/judge.cpp -std=c++11 -O2 -Wall -o ./debug/judge
echo compile_judge_complete
g++ ./src/search.cpp -std=c++11 -O2 -Wall -o ./debug/search
echo compile_search_complete

# Execute this program.
./debug/judge 7122 > ./debug/log_ju.txt &
./debug/search 127.0.0.1 7122 > ./debug/log_p1.txt &
./debug/search 127.0.0.1 7122 > ./debug/log_p2.txt &
