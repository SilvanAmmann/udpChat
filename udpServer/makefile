default: all

# Compiler and linker settings
CC = aarch64-linux-gnu-g++
PYTHON_CFLAGS = $(shell pkg-config --cflags python3)
PYTHON_LIBS = $(shell pkg-config --libs python3)
LINK_TARGET = $(OBJ_DIR)udpserver
DEPEND_OPTS = -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)"
SRC_DIR = ./
OBJ_DIR = ./

# Object files
OBJS = \
	$(OBJ_DIR)main.o

# List of dependency files
C_DEPS = \
  $(OBJ_DIR)main.d

# Include generated dependency files (only if not clean target)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

all: $(LINK_TARGET)
	@echo All done!

clean:
	-@rm -f $(LINK_TARGET)
	-@rm -f $(OBJS)
	-@rm -f $(C_DEPS)
	@echo Clean done!

# Pattern rule to compile a file	
# $@ matches the target
# $< matches the first dependent
$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	$(CC) $(DEPEND_OPTS) -g3 -c "$<" -o "$@" $(PYTHON_CFLAGS)

$(LINK_TARGET): $(OBJS)
	$(CC) $(OBJS) -ggdb -o $@ $(PYTHON_LIBS) -lgpiod
