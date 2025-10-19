SHELL := bash
.ONESHELL:
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-print-directory

.DEFAULT_GOAL = help

INIT := .initialized
BIN_DIR := bin
OBJ_DIR := obj

TARGET = bin/dbview

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

$(INIT):
	@mkdir -p $(BIN_DIR)
	mkdir -p $(OBJ_DIR)
	touch $@

.PHONY: init
init: ## Setup build environment
init: $(INIT)

clean: ## Remove build directory
	@rm -f obj/*.o
	rm -f bin/*
	rm -f *.db
	rm -f $(INIT)

$(TARGET): $(OBJ)
	@gcc -o $@ $?

obj/%.o : src/%.c
	@gcc -c $< -o $@ -Iinclude

run: ## Run example
run: clean $(INIT) $(TARGET)
	@echo -e "\nRunning $(TARGET)\n"
	./$(TARGET) -f mynewdb.db -n
	./$(TARGET) -f ./mynewdb.db -a "Timmy H.,123 Sheshire Ln.,120"
	./$(TARGET) -f ./mynewdb.db -a "Timmy H.,123 Sheshire Ln.,120"
	./$(TARGET) -f ./mynewdb.db -a "Timmy H.,123 Sheshire Ln.,120"
	hexyl -g2 mynewdb.db

profile: ## Run with profiling tools
profile: $(OBJ)
	@gcc -g -o $(TARGET) $(OBJ) -Iinclude
	valgrind -s --track-origins=yes --leak-check=full $(TARGET)

help: ## Show help message
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m\033[0m\n"} /^[$$()% 0-9a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

