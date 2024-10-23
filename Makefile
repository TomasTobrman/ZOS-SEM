# ============================================================================ #
#   KIV/ZOS - Tirea File System                                                #
# ============================================================================ #

PROJECT         := tfs

# ---- DIRECTORIES ----------------------------------------------------------- #
BINARY_FOLDER   := ./bin
OBJECT_FOLDER   := ./obj
SOURCE_FOLDER   := ./src
DIRECTORIES     := $(shell find $(SOURCE_FOLDER) -type d)

# ---- FILES ----------------------------------------------------------------- #
SOURCE_FILES    := $(shell find . -name *.c)
OBJECT_FILES    := $(SOURCE_FILES:%=$(OBJECT_FOLDER)/%.o)

# ---- FLAGS ----------------------------------------------------------------- #
# DEFINES         := -DUSE_HISTORY
DEFINES         :=
COMPILER_FLAGS  := -Wall -Werror -g
LINKER_FLAGS    :=
INCLUDE_FLAGS   := -I$(SOURCE_FOLDER)

# ---- TARGETS --------------------------------------------------------------- #
CC              := gcc

.PHONY: all
all: scaffold compile link gen_compile_flags

.PHONY: scaffold
scaffold:
	@mkdir -p $(BINARY_FOLDER)
	@mkdir -p $(addprefix $(OBJECT_FOLDER)/,$(DIRECTORIES))

.PHONY: link
link: scaffold $(OBJECT_FILES)
	@echo linking "$(PROJECT)"...
	@$(CC) $(OBJECT_FILES) $(LINKER_FLAGS) -o $(BINARY_FOLDER)/$(PROJECT)

.PHONY: compile
compile:
	@echo --- Building "$(PROJECT)" Executable ---
-include $(OBJECT_FILES:.o=.d)

.PHONY: clean
clean:
	@echo --- Cleaning "$(PROJECT)" ---
	@rm -rf $(BINARY_FOLDER)/$(PROJECT)
	@rm -rf $(OBJECT_FOLDER)/*

$(OBJECT_FOLDER)/%.c.o: %.c
	@echo "  "$<...
	@$(CC) $< $(COMPILER_FLAGS) $(INCLUDE_FLAGS) $(DEFINES) -c -o $@

-include $(OBJECT_FILES:.o=.d)

.PHONY: gen_compile_flags
gen_compile_flags:
	@echo $(INCLUDE_FLAGS) $(DEFINES) | tr " " "\n" > ./compile_flags.txt

# ---------------------------------------------------------------------------- #
