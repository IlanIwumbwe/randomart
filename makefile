SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
FLAGS = -Wextra -Wall
TARGET = randomart

%.o : %.c
	gcc $(FLAGS) -c $< -o $@

$(TARGET) : $(OBJS)
	gcc $(FLAGS) -o $@ $<

all: $(TARGET)

.PHONY: clean debug

clean:
	rm -rf $(TARGET) $(OBJS)

debug:
	$(MAKE) FLAGS="-Wextra -Wall -DDEBUG -g"