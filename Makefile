CC=gcc
CFLAGS_release = -O3
CFLAGS_debug = -O0 -g
BUILD=release

AVX512=-mno-avx512f

CFLAGS=-Wall -std=c17 ${CFLAGS_${BUILD}} ${AVX512} -mbmi2  -mavx2 -march=native -fopenmp -I ./src/inc -I ./Puzzle-A-Day-Solver/src/inc  -Lbuild -fPIC 

BUILD_DIR=build
$(shell mkdir -p ${BUILD_DIR})
SRC_DIR=src


.PHONY: all
all: dependencies
	$(MAKE) main eval.so eval exhaustive exhaustive.so



.PHONY: dependencies
dependencies:
	$(MAKE) -C Puzzle-A-Day-Solver AVX512=${AVX512} BUILD=${BUILD} BUILD_DIR=../$(BUILD_DIR)





$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

main: $(BUILD_DIR)/solver.o $(BUILD_DIR)/board.o $(BUILD_DIR)/piece.o $(BUILD_DIR)/problem.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/printing.o $(BUILD_DIR)/gene.o $(BUILD_DIR)/main.o
	$(CC) $(CFLAGS) -o $@ $^

eval.so: $(BUILD_DIR)/solver.o $(BUILD_DIR)/board.o $(BUILD_DIR)/piece.o $(BUILD_DIR)/problem.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/printing.o $(BUILD_DIR)/gene.o $(BUILD_DIR)/eval.o
	$(CC) $(CFLAGS) -shared -o $@ $^


eval: $(BUILD_DIR)/solver.o $(BUILD_DIR)/board.o $(BUILD_DIR)/piece.o $(BUILD_DIR)/problem.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/printing.o $(BUILD_DIR)/gene.o $(BUILD_DIR)/eval.o
	$(CC) $(CFLAGS) -o $@ $^

exhaustive: $(BUILD_DIR)/solver.o $(BUILD_DIR)/board.o $(BUILD_DIR)/piece.o $(BUILD_DIR)/problem.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/printing.o $(BUILD_DIR)/gene.o  $(BUILD_DIR)/exhaustive.o
	$(CC) $(CFLAGS) -o $@ $^


exhaustive.so: $(BUILD_DIR)/solver.o $(BUILD_DIR)/board.o $(BUILD_DIR)/piece.o $(BUILD_DIR)/problem.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/printing.o $(BUILD_DIR)/gene.o $(BUILD_DIR)/exhaustive.o
	$(CC) $(CFLAGS) -shared -o $@ $^

.PHONY: clean
clean:
	rm -f *.o build/* main
