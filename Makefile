CC      := gcc
CXX     := g++
INCLUDE := -Iinclude
LIBS    := 
CARGS   := $(INCLUDE) -ggdb -Wall -Wextra -pedantic -std=c++23 #-Werror
OUT     := run

objects += main.o

build: $(addprefix obj/, $(objects))
	@mkdir -p $(dir ./$(OUT))
	$(CXX) $(CARGS) -o ./$(OUT) $^ $(LIBS)

obj/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c $(CARGS) -o $@ $^

run: build
	./$(OUT)

gdb: build
	gdb ./$(OUT)

clean:
	rm $(OUT)
	rm -r ./obj