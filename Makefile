# Compiler and flags
CXX = clang++-15
CXXFLAGS = -std=c++17 -Iinclude -Ithird_party -Wall -Wextra -pedantic -g
LDFLAGS = -lm -lpthread -ldl -fuse-ld=lld-15
EXECUTABLE = bpm

# Directories
SRCDIR = src
OBJDIR = obj

# Find all .cpp files in the source directory
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
# Create a list of object files in the object directory
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

# The default target, 'all', should be first.
.PHONY: all
all: $(EXECUTABLE)

# Rule to link the executable.
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

# Rule to compile a .cpp file into a .o file.
# The `| $(OBJDIR)` part is an order-only prerequisite, ensuring the obj dir exists.
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to create the object directory.
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Phony target to clean up the build artifacts.
.PHONY: clean
clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJDIR) $(EXECUTABLE)

# Phony target for installing the executable
.PHONY: install
install: $(EXECUTABLE)
	@echo "Installing $(EXECUTABLE) to /usr/local/bin"
	@sudo install $(EXECUTABLE) /usr/local/bin/

# Phony target for uninstalling the executable
.PHONY: uninstall
uninstall:
	@echo "Uninstalling $(EXECUTABLE) from /usr/local/bin"
	@sudo rm -f /usr/local/bin/$(EXECUTABLE)
