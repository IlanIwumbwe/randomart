SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
FLAGS = -Wextra -Wall -Wswitch-enum
TARGET = randomart

%.o : %.c
	gcc $(FLAGS) -c $< -o $@

$(TARGET) : $(OBJS)
	gcc $(FLAGS) -o $@ $< -lm

all: $(TARGET)

.PHONY: clean debug

clean:
	rm -rf $(TARGET) $(OBJS)

debug:
	$(MAKE) FLAGS="-Wextra -Wall -Wswitch-enum -DDEBUG -g"