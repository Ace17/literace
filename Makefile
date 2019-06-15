BIN?=bin

CXXFLAGS+=-O3

SRCS:=\
	game.cpp \
	main.cpp \

PKGS+=sdl2
CXXFLAGS+=$(shell pkg-config $(PKGS) --cflags)
LDFLAGS+=$(shell pkg-config $(PKGS) --libs)

$(BIN)/literace.exe: $(SRCS:%=$(BIN)/%.o)
	@mkdir -p $(dir $@)
	$(CXX) -o "$@" $(LDFLAGS) $^

$(BIN)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c $(CXXFLAGS) -o "$@" $<

clean:
	rm -rf $(BIN)
