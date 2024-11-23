# Compiler and flags
CC = gcc
CFLAGS = -Wall -g -Iinclude


# Directories
SRC_DIR = src
INC_DIR = include

# Source files and object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o)

# Target executable
TARGET = filesystem

# Build target
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Compile .c files into .o files
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/*.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)
