BIN?=bin

CXXFLAGS+=-O3

SRCS:=\
	game.cpp \
	input.cpp \
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
	@$(CXX) -MM -MT "$@" -MP -c $(CXXFLAGS) -o "$@.dep" $<

clean:
	rm -rf $(BIN)

include $(shell test -d $(BIN) && find $(BIN) -name "*.dep")
