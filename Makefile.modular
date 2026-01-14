# Makefile.modular - Modular TestGen Build System with Test API Support
# Supports multiple web applications with independent configurations

CXX = g++

# Curl paths (for HTTP requests)
CURL_PREFIX := /usr/local/opt/curl
CURL_LDFLAGS := -L$(CURL_PREFIX)/lib
CURL_CPPFLAGS := -I$(CURL_PREFIX)/include

# ═══════════════════════════════════════════════════════════════════
# UPDATED: Added -Iconfig/test_api for Test API headers
# ═══════════════════════════════════════════════════════════════════
CXXFLAGS = -std=c++17 -Wall -Wextra -I. -Iconfig -Iconfig/test_api -g -O2 $(CURL_CPPFLAGS)

# Libs for linking
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

# API implementation sources
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

# ═══════════════════════════════════════════════════════════════════
# Header dependencies (for reference - helps with rebuilds)
# ═══════════════════════════════════════════════════════════════════
TEST_API_HEADERS = \
    config/test_api/TestAPIBase.hpp \
    config/test_api/PesuFoodsTestAPI.hpp \
    config/test_api/RestaurantTestAPI.hpp \
    SymbolicEngine/SymbolicVisitorTestAPIHandler.hpp

SYMBOLIC_ENGINE_HEADERS = \
    SymbolicEngine/TestGenSymbolicEnv.hpp \
    SymbolicEngine/TestGenSymbolicVisitor.hpp \
    SymbolicEngine/TestGenDriver.hpp

# Build targets
.PHONY: all clean basic symbolic test help

all: symbolic

basic: $(TARGET_BASIC)

symbolic: $(TARGET_SYMBOLIC)

# Basic build (without symbolic engine)
$(TARGET_BASIC): $(MODULAR_SOURCES) $(CODEGEN_SOURCES)
	@echo "Building modular basic TestGen..."
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)
	@echo "✓ Built: $@"

# Symbolic build (with symbolic engine + Test API)
$(TARGET_SYMBOLIC): $(ALL_SYMBOLIC_SOURCES)
	@echo "Building modular TestGen with SymbolicEngine + Test API..."
	$(CXX) $(CXXFLAGS) -ISymbolicEngine -DUSE_SYMBOLIC_ENGINE $^ -o $@ $(LIBS)
	@echo "✓ Built: $@"

# Test target - verify Test API headers exist
test:
	@echo "Checking Test API files..."
	@test -f config/test_api/TestAPIBase.hpp && echo "  ✓ TestAPIBase.hpp" || echo "  ✗ TestAPIBase.hpp MISSING"
	@test -f config/test_api/PesuFoodsTestAPI.hpp && echo "  ✓ PesuFoodsTestAPI.hpp" || echo "  ✗ PesuFoodsTestAPI.hpp MISSING"
	@test -f config/test_api/RestaurantTestAPI.hpp && echo "  ✓ RestaurantTestAPI.hpp" || echo "  ✗ RestaurantTestAPI.hpp MISSING"
	@test -f SymbolicEngine/SymbolicVisitorTestAPIHandler.hpp && echo "  ✓ SymbolicVisitorTestAPIHandler.hpp" || echo "  ✗ SymbolicVisitorTestAPIHandler.hpp MISSING"
	@echo ""
	@echo "Checking mongosh availability..."
	@which mongosh > /dev/null && echo "  ✓ mongosh found" || echo "  ✗ mongosh NOT FOUND (required for Test API)"

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(TARGET_BASIC) $(TARGET_SYMBOLIC)
	rm -f *.o jsCodeGenerator/*.o SymbolicEngine/*.o config/restaurant/*.o config/pesu_foods/*.o
	rm -f genctc_query.smt2 final_constraints.smt2 z3_result.txt
	@echo "✓ Clean complete"

help:
	@echo "Makefile.modular - Modular TestGen Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make            - Build symbolic version (default)"
	@echo "  make basic      - Build basic (non-symbolic) version"
	@echo "  make symbolic   - Build with symbolic execution + Test API"
	@echo "  make test       - Check that all required files exist"
	@echo "  make clean      - Remove all build artifacts"
	@echo ""
	@echo "Usage:"
	@echo "  ./$(TARGET_SYMBOLIC) --list-apps"
	@echo "  ./$(TARGET_SYMBOLIC) --app restaurant --list-scenarios"
	@echo "  ./$(TARGET_SYMBOLIC) --app restaurant --scenario restaurant_scenario_1 --symbolic"
	@echo ""
	@echo "Prerequisites:"
	@echo "  - MongoDB running locally"
	@echo "  - mongosh installed (for Test API)"
	@echo "  - Restaurant backend: cd restaurant-backend-api && PORT=5002 npm start"
