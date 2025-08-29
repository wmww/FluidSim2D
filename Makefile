CXX = g++
CXXFLAGS = -std=c++11 -O
PROJECT_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
$(info    PROJECT_DIR is $(PROJECT_DIR))
SFML_DIR = $(PROJECT_DIR)/deps/SFML-2.6.2
INCLUDES = -I$(SFML_DIR)/include
LIBS = -L$(SFML_DIR)/lib -Wl,-rpath,$(SFML_DIR)/lib -lsfml-window -lsfml-graphics -lsfml-system
DEFINES = -DFONT_PATH='"$(PROJECT_DIR)/deps/Ubuntu-R.ttf"'

SRCDIR = src
OBJDIR = build
SOURCES = $(SRCDIR)/Fluidsim.cpp $(SRCDIR)/Graphics.cpp $(SRCDIR)/main.cpp $(SRCDIR)/Particle.cpp $(SRCDIR)/ShapeLoader.cpp Widap_includes/WidapImage.cpp
OBJECTS = $(OBJDIR)/Fluidsim.o $(OBJDIR)/Graphics.o $(OBJDIR)/main.o $(OBJDIR)/Particle.o $(OBJDIR)/ShapeLoader.o $(OBJDIR)/WidapImage.o

TARGET = $(OBJDIR)/FluidSim2D

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(OBJDIR)/WidapImage.o: Widap_includes/WidapImage.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

.PHONY: clean
