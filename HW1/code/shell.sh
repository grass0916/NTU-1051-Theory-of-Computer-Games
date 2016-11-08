# Move to the project root.
BASEDIR=$(dirname "$0")
cd $BASEDIR

# Create the debug folder.
rm -rf ./debug
mkdir -p ./debug

# Compiler the program.
g++ ./src/main.cpp ./src/elapsed_time.cpp -std=c++14 -o ./debug/out.o

# Generate the input data.
cd ./data
rm tcga2016-question.txt tcga2016-solution.txt
# <rows, column> <amount> <max_filled> <min_filled> <seed>
python ./boardgen.py 15 1000 0.5 0.3 12345

# Execute this program.
cd ../
# <input_data> <columns> <rows> <amount> <method> <times_for_average>
./debug/out.o tcga2016-question.txt 15 15 25 DFS 10
./debug/out.o tcga2016-question.txt 15 15 25 BFS 10
