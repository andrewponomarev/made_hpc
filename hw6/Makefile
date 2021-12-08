CC=nvcc
CFLAGS=

SRC=src
BIN=bin

SRC_FILES=$(wildcard $(SRC)/*.cu)
TARGETS=$(patsubst $(SRC)/%.cu,$(BIN)/%,$(SRC_FILES))

all: $(TARGETS)

$(BIN):
	mkdir -p $(BIN)

$(TARGETS): | $(BIN)

$(BIN)/%: $(SRC)/%.cu
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -r $(BIN)
