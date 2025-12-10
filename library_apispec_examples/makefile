# Makefile.modular - Modular TestGen Build System
# Supports multiple web applications with independent configurations
CXX = g++

# ✅ ADD: curl paths
CURL_PREFIX := /usr/local/opt/curl
CURL_LDFLAGS := -L$(CURL_PREFIX)/lib
CURL_CPPFLAGS := -I$(CURL_PREFIX)/include

# ✅ UPDATE: Add CURL_CPPFLAGS to CXXFLAGS
CXXFLAGS = -std=c++17 -Wall -Wextra -I. -Iconfigs -g -O2 $(CURL_CPPFLAGS)

# ✅ ADD: LIBS for linking
LIBS := $(CURL_LDFLAGS) -lcurl

# Target executables
TARGET_BASIC = testgen_modular_basic
TARGET_SYMBOLIC = testgen_modular_symbolic

# Core modular sources (new architecture)
MODULAR_SOURCES = \
    WebAppConfig.cpp \
    ModularTestGen.cpp \
    test_modular_application.cpp

# Code generation sources (existing)
CODEGEN_SOURCES = \
    jsCodeGenerator/jsCodeGen.cpp

API_IMPL_SOURCES = \
    config/pesu_foods/api.cpp \
    config/restaurant/api.cpp

# Symbolic engine sources
SYMBOLIC_ENGINE_SOURCES = \
    SymbolicEngine/TestGenSymbolicEnv.cpp \
    SymbolicEngine/TestGenSymbolicVisitor.cpp \
    SymbolicEngine/TestGenDriver.cpp

# All sources for symbolic build
ALL_SYMBOLIC_SOURCES = $(MODULAR_SOURCES) $(CODEGEN_SOURCES) $(SYMBOLIC_ENGINE_SOURCES) $(API_IMPL_SOURCES)

# Build targets
.PHONY: all clean basic symbolic

all: symbolic

basic: $(TARGET_BASIC)

symbolic: $(TARGET_SYMBOLIC)

# ✅ UPDATE: Add $(LIBS) at the end
$(TARGET_BASIC): $(MODULAR_SOURCES) $(CODEGEN_SOURCES)
	@echo "Building modular basic TestGen..."
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)
	@echo "Built: $@"

# ✅ UPDATE: Add $(LIBS) at the end
$(TARGET_SYMBOLIC): $(ALL_SYMBOLIC_SOURCES)
	@echo "Building modular TestGen with SymbolicEngine..."
	$(CXX) $(CXXFLAGS) -ISymbolicEngine -DUSE_SYMBOLIC_ENGINE $^ -o $@ $(LIBS)
	@echo "Built: $@"

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(TARGET_BASIC) $(TARGET_SYMBOLIC)
	rm -f *.o jsCodeGenerator/*.o SymbolicEngine/*.o config/restaurant/*.o
	@echo "Clean complete"

.PHONY: help
help:
	@echo "Makefile.modular - Modular TestGen Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make basic          - Build basic (non-symbolic) version"
	@echo "  make symbolic       - Build with symbolic execution engine + API support"
	@echo "  make clean          - Remove all build artifacts"
	@echo ""
	@echo "Usage:"
	@echo "  ./$(TARGET_BASIC) --app pesu_foods --scenario pesu_foods_scenario_1"
	@echo "  ./$(TARGET_SYMBOLIC) --app pesu_foods --scenario pesu_foods_scenario_1 --symbolic"
