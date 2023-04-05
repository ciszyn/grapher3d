src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)

LDFLAGS = -lGL -lGLEW -lsfml-graphics -lsfml-system -lsfml-window

grapher: $(obj)
	g++ -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) myprog
