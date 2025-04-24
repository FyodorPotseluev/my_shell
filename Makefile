PROJECT := my_shell

# Variables for paths of source, header, and test files
INC_DIR := ./include
SRC_DIR := ./src
SRCMODULES := $(wildcard $(SRC_DIR)/*.c)
TEST_DIR := ./test

# Variables for paths of object files and binary targets
BUILD_DIR := ./build
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
EXECUTABLE := $(BIN_DIR)/$(PROJECT)
BUILD_DIRS := $(OBJ_DIR) $(BIN_DIR)
OBJMODULES := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCMODULES))

# C compiler configuration
CC = gcc # using gcc compiler
CFLAGS = -Wall -Wextra -g3 -O0 -Iinclude -fsanitize=address,undefined
# CFLAGS options:
# -Wall		Warnings: all - display every single warning
# -Wextra	Enable some extra warning flags that are not enabled by -Wall
# -g3		Compile with verbose debugging information, including
#		preprocessor macros and additional metadata;
# -O0		Disable compilation optimizations
# -Iinclude	Add the directory /include to the list of directories to be
#  		searched for header files during preprocessing.
# -fsanitize=	Enable sanitizers, which inject extra checks into the code
#		compile time, preparing it to catch potential issues at runtime;
#	address
#		This sanitizer detects memory-related errors such as buffer
#		overflows, heap-use-after-free and stack-use-after-return;
#	undefined
#		This sanitizer detects undefined behavior.

# Conditionally add additional flags based on the value of D
ifeq ($(D),PRINT_TOKENS_MODE)
    CFLAGS += -D PRINT_TOKENS_MODE
else
    CFLAGS += -D EXEC_MODE
    D = EXEC_MODE
endif

all: $(EXECUTABLE)

# Display useful goals in this Makefile
help:
	@echo "Try one of the following make goals:"
	@echo " > (no goals) compile using sanitizers. Execute programs;"
	@echo " > D=PRINT_TOKENS_MODE - compile using sanitizers. Print all the tokens;"
	@echo " > readme - project's documentation"
	@echo " > run - execute the project"
	@echo " > print_tokens_test"
	@echo "     - run the project's integration test (correct string splitting into tokens)"
	@echo " > session_test"
	@echo "     - run the project's integration test (correct \`my_shell\` session)"
	@echo " > memcheck_print_tokens_test"
	@echo " > memcheck_session_test"
	@echo "     - memory check the print_tokens_test"
	@echo " > debug - begin a gdb process for the executable"
	@echo " > leak_search - run the project under valgrind"
	@echo " > clean - delete build files in project"
	@echo " > variables - print Makefile's variables"

# Build the project by combining all object files
$(EXECUTABLE): $(OBJMODULES) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -lm -o $@

# Build object files from sources in a template pattern
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -lm -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCMODULES)
	$(CC) -MM -Iinclude -D $(D) $^ > $@

readme:
ifdef EDITOR
	$(EDITOR) README.txt
else
	xdg-open README.txt
endif

run: $(EXECUTABLE)
	@$(EXECUTABLE)

print_tokens_test:
	$(TEST_DIR)/print_tokens_test.sh

session_test:
	$(TEST_DIR)/session_test.sh

memcheck_session_test:
	$(TEST_DIR)/memcheck_session_test.sh

memcheck_print_tokens_test:
	$(TEST_DIR)/memcheck_print_tokens_test.sh

debug:
	gdb --args $(EXECUTABLE)

leak_search:
	valgrind --tool=memcheck --leak-check=full --errors-for-leak-kinds=definite,indirect,possible --show-leak-kinds=definite,indirect,possible $(EXECUTABLE)

clean:
	rm -f $(OBJ_DIR)/* $(EXECUTABLE)

variables:
	@echo "PROJECT =" $(PROJECT)
	@echo
	@echo "# Variables for paths of source, header, and test files"
	@echo "INC_DIR =" $(INC_DIR)
	@echo "SRC_DIR =" $(SRC_DIR)
	@echo "SRCMODULES =" $(SRCMODULES)
	@echo "TEST_DIR =" $(TEST_DIR)
	@echo
	@echo "# Variables for paths of object files and binary targets"
	@echo "BUILD_DIR =" $(BUILD_DIR)
	@echo "OBJ_DIR =" $(OBJ_DIR)
	@echo "BIN_DIR =" $(BIN_DIR)
	@echo "EXECUTABLE =" $(EXECUTABLE)
	@echo "BUILD_DIRS =" $(BUILD_DIRS)
	@echo "OBJMODULES =" $(OBJMODULES)
	@echo
	@echo "# C compiler configuration"
	@echo "CC =" $(CC)
	@echo "CFLAGS =" $(CFLAGS)
