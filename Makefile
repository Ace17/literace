BIN?=bin

CXXFLAGS+=-g
LDFLAGS+=-g

SRCS:=\
	game.cpp \
	display.cpp \
	input.cpp \
	main.cpp \

PKGS+=sdl2 gl
PKG_CFLAGS+=$(shell pkg-config $(PKGS) --cflags)
PKG_LDFLAGS+=$(shell pkg-config $(PKGS) --libs)

$(BIN)/literace.exe: $(SRCS:%=$(BIN)/%.o)
	@mkdir -p $(dir $@)
	$(CXX) -o "$@" $(LDFLAGS) $^ $(PKG_LDFLAGS)

$(BIN)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c $(CXXFLAGS) -o "$@" $< $(PKG_CFLAGS)
	@$(CXX) -MM -MT "$@" -MP -c $(CXXFLAGS) -o "$@.dep" $< $(PKG_CFLAGS)

clean:
	rm -rf $(BIN)

include $(shell test -d $(BIN) && find $(BIN) -name "*.dep")
