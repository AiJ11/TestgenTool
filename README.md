# TestGen: Specification-Based API Test Generation

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Z3](https://img.shields.io/badge/Z3-SMT%20Solver-green.svg)](https://github.com/Z3Prover/z3)

**TestGen** is a specification-based test generation framework for REST APIs that uses symbolic execution and SMT constraint solving to automatically generate concrete, executable test cases from high-level formal specifications.

## Overview

Unlike schema-based tools (Schemathesis, RESTler) that test APIs in isolation, TestGen reasons about **multi-step stateful workflows** involving:
- Authentication and role-based access control
- Cross-endpoint entity dependencies
- Complex state machines and business logic

TestGen takes a formal API specification written in a custom DSL and produces executable test cases that exercise realistic end-to-end scenarios.

## Key Features

- **Symbolic Execution Engine**: Tracks symbolic state across API call sequences
- **Z3 SMT Solver Integration**: Generates concrete values satisfying path constraints
- **Multi-Actor Support**: Handles role-based workflows (e.g., Seller→Buyer, Owner→Customer→Agent)
- **Authentication Management**: Automatically manages JWT tokens across role transitions
- **Entity Dependency Resolution**: Resolves dynamic IDs and cross-endpoint references
- **Infeasibility Detection**: Identifies and prunes logically impossible test sequences (UNSAT)
- **Live Backend Execution**: Executes generated tests against running applications

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        TestGen Framework                         │
├─────────────────────────────────────────────────────────────────┤
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐      │
│  │    Spec      │    │   Symbolic   │    │     Z3       │      │
│  │   Parser     │───▶│   Executor   │───▶│   Solver     │      │
│  └──────────────┘    └──────────────┘    └──────────────┘      │
│         │                   │                   │               │
│         ▼                   ▼                   ▼               │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐      │
│  │  Test String │    │  Constraint  │    │   Concrete   │      │
│  │   Parsing    │    │ Accumulation │    │    Values    │      │
│  └──────────────┘    └──────────────┘    └──────────────┘      │
│                             │                                   │
│                             ▼                                   │
│                    ┌──────────────┐                             │
│                    │  HTTP Test   │                             │
│                    │  Execution   │                             │
│                    └──────────────┘                             │
└─────────────────────────────────────────────────────────────────┘
```

## Prerequisites

- **C++17** compatible compiler (g++ 9+ or clang++ 10+)
- **Z3 SMT Solver** (v4.8+)
- **libcurl** for HTTP requests
- **nlohmann/json** for JSON parsing (bundled in `includes/json.hpp`)

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y build-essential libz3-dev libcurl4-openssl-dev cmake
```

**macOS (Homebrew):**
```bash
brew install z3 curl
```

## Installation

```bash
# Clone the repository
git clone <repository-url>
cd TestgenTool

# Build the project
make

# Run the application
make run
```

### Build Commands

| Command | Description |
|---------|-------------|
| `make` | Build the project |
| `make run` | Build and run the project |
| `make clean` | Remove built files |
| `make rebuild` | Clean and rebuild |
| `make help` | Show help message |

### Platform Configuration

The Makefile is configured for **macOS (Homebrew)** by default. For Linux, edit the `makefile` and uncomment the Linux paths:

```makefile
# Linux (uncomment if using Linux)
INCLUDES = -I/usr/include
LDFLAGS = -L/usr/lib
```

## Usage

### Test Execution Modes

TestGen supports three execution modes, selectable via `TestMode` in `test_libapplication.cpp`:

| Mode | Description |
|------|-------------|
| `ORIGINAL` | Runs genATC only — generates the Abstract Test Case without rewriting |
| `REWRITE_ONLY` | Runs genATC + RewriteGlobalsVisitor — no backend connection required |
| `FULL_PIPELINE` | Complete pipeline: genATC + Rewrite + Symbolic Execution + live backend HTTP calls |

### Running Tests

1. **Start the target backend** (required for `FULL_PIPELINE` mode only)
2. **Select the application** in `test_libapplication.cpp` by uncommenting the relevant block in `main()`
3. **Build and run:**
   ```bash
   make run
   ```

Alternatively, use the provided shell script:
```bash
./run.sh
```

### Backend URLs

Each application runs on a fixed default port. Start the corresponding backend before running tests in `FULL_PIPELINE` mode.

| Application | Default URL |
|-------------|-------------|
| Restaurant | `http://localhost:5002` |
| Ecommerce | `http://localhost:3000` |
| Library | `http://localhost:8080` |
| GhostSocket | `http://localhost:4002` |
| Serveez | `http://localhost:8083` |
| TripVault | `http://localhost:4001` |

### Test Output Files

All test outputs are stored in the `all_test_files/` directory.

| Application | Output Files |
|-------------|-------------|
| Restaurant | `test1.txt` – `test25.txt` (25 tests) |
| Ecommerce | `test1ecom.txt` – `test30ecom.txt` (30 tests) |
| Library | `lib1.txt` – `lib25.txt` (25 tests) |
| GhostSocket | `ghostsocket_ALL25_tests.txt` |
| Serveez | `serveez_all25_tests.txt` |
| TripVault | `tripvault_ALL25_tests.txt` |

Bug detection test outputs (8 injected bugs per application) are stored as `bugtest_*.txt` files in the same directory.

## Project Structure

```
TestgenTool/
│
├── makefile                        # Build configuration
├── run.sh                          # Convenience run script
├── test_libapplication.cpp         # Main entry point — all test runners
├── algo.cpp / algo.hpp             # genATC algorithm implementation
├── ast.cc / ast.hh                 # Abstract Syntax Tree
├── ast_tests.cpp                   # AST unit tests
├── ast_workarounds.hpp             # AST compatibility helpers
├── atc_ast.hpp                     # ATC-specific AST extensions
├── astvisitor.cc / astvisitor.hh   # AST visitor base
├── printvisitor.cc / printvisitor.hh   # AST pretty-printer
├── clonevisitor.cc / clonevisitor.hh   # AST deep-clone visitor
├── CloneVisitor.hpp                # Clone visitor header (alternate)
├── cloneHelper.hpp                 # Clone utility helpers
├── rewrite_globals_visitor.cc / .hh    # Globals rewrite pass (test API layer)
├── env.cc / env.hh                 # Environment / symbolic state management
├── typemap.cc / typemap.hh         # Type mapping utilities
├── symvar.cc / symvar.hh           # Symbolic variable definitions
├── location.hh                     # Source location tracking
│
├── includes/
│   └── json.hpp                    # Bundled nlohmann/json
│
├── specs/                          # Formal API specifications
│   ├── RestaurantSpec.cpp / .hpp   # Restaurant (Food Ordering) spec
│   ├── EcommerceSpec.cpp / .hpp    # E-commerce spec
│   ├── LibrarySpec.cpp / .hpp      # Library Management spec
│   ├── GhostSocketSpec.cpp / .hpp  # GhostSocket (WebSocket/IoT) spec
│   ├── ServeezSpec.cpp / .hpp      # Serveez (Service Booking) spec
│   └── TripVaultSpec.cpp / .hpp    # TripVault (Trip & Expense) spec
│
├── see/                            # Symbolic Execution Engine
│   ├── see.cc / see.hh             # SEE core
│   ├── solver.cc / solver.hh       # Abstract solver interface
│   ├── z3solver.cc / z3solver.hh   # Z3 SMT solver integration
│   ├── functionfactory.cc / .hh    # Base function factory
│   ├── httpclient.cc / httpclient.hh   # HTTP client for live execution
│   ├── restaurantfunctionfactory.cc / .hh
│   ├── ecommercefunctionfactory.cc / .hh
│   ├── libraryfunctionfactory.cc / .hh
│   ├── ghostsocketfunctionfactory.cc / .hh
│   ├── serveezfunctionfactory.cc / .hh
│   └── tripvaultfunctionfactory.cc / .hh
│
├── tester/
│   ├── tester.cc / tester.hh       # Concrete Test Case (CTC) generator
│
├── unit_tests/                     # Unit tests for core components
│   ├── test.cpp
│   ├── test_decl_clone.cpp
│   └── test_program.cpp
│
├── all_test_files/                 # Generated test output files
│   ├── test1.txt – test25.txt      # Restaurant tests
│   ├── test1ecom.txt – test30ecom.txt  # Ecommerce tests
│   ├── lib1.txt – lib25.txt        # Library tests
│   ├── ghostsocket_ALL25_tests.txt # GhostSocket tests
│   ├── serveez_all25_tests.txt     # Serveez tests
│   ├── tripvault_ALL25_tests.txt   # TripVault tests
│   └── bugtest_*.txt               # Bug detection test outputs (all apps)
│
└── mutation_testing/               # Mutation testing study artifact
    ├── mutationOperator_report.txt # Full catalogue of all 104 mutants
    ├── restaurant/
    │   ├── MUTATION_REPORT.txt
    │   └── mutants/
    ├── ecommerce/
    │   ├── MUTATION_REPORT.txt
    │   └── mutants/
    ├── library/
    │   ├── MUTATION_REPORT.txt
    │   └── mutants/
    ├── ghostsocket/
    │   ├── MUTATION_REPORT.txt
    │   └── mutants/
    ├── serveez/
    │   ├── MUTATION_REPORT.txt
    │   └── mutants/
    └── tripvault/
        ├── MUTATION_REPORT.txt
        └── mutants/
```

### Module Descriptions

| Module | Description |
|--------|-------------|
| **test_libapplication.cpp** | Main entry point; contains all six application test runners |
| **algo** | genATC — generates Abstract Test Cases (ATCs) from test strings |
| **ast** | Abstract Syntax Tree used to represent specifications and programs |
| **rewrite_globals_visitor** | Rewrites global state references for the symbolic execution layer |
| **env** | Symbolic environment: tracks variable bindings and state maps |
| **symvar** | Symbolic variable representation and operations |
| **specs/** | Formal API specifications for each subject application |
| **see/** | Symbolic Execution Engine with Z3 SMT solver and HTTP client |
| **tester/** | genCTC — generates Concrete Test Cases from symbolic execution results |
| **unit_tests/** | Unit tests for AST and core components |
| **all_test_files/** | Saved test execution outputs for all six applications |
| **mutation_testing/** | Mutation testing reports and mutant output for all six applications |

## Supported Applications

TestGen has been evaluated on six real-world web applications:

| Application | Language | Roles | Tests Generated |
|-------------|----------|-------|-----------------|
| Restaurant (Food Ordering) | Node.js | Owner, Customer, Agent | 25 |
| Ecommerce | Node.js | Seller, Buyer | 30 |
| Library Management | Java (Spring Boot) | Admin, Student | 25 |
| GhostSocket (WebSocket/IoT) | Node.js | Owner, User | 25 |
| Serveez (Service Booking) | Java (Spring Boot) | Provider, Customer | 25 |
| TripVault (Trip & Expense) | Node.js | User (multi-member trips) | 25 |

## Mutation Testing

TestGen was evaluated against **104 mutants** injected across all six backends (10 per app for the first five, 54 for GhostSocket). All 104 mutants were detected (100% mutation score).

| Application | Mutants | Detected | Score |
|-------------|---------|----------|-------|
| Restaurant | 10 | 10 | 100% |
| Ecommerce | 10 | 10 | 100% |
| Library | 10 | 10 | 100% |
| Serveez | 10 | 10 | 100% |
| TripVault | 10 | 10 | 100% |
| GhostSocket | 54 | 54 | 100% |

See `mutation_testing/mutationOperator_report.txt` for the full catalogue of every injected mutant including the fault type, source location, and basis for site selection.

## Comparison with Other Tools

| Feature | TestGen | RESTler | Schemathesis |
|---------|---------|---------|--------------|
| Multi-step workflows | Yes | No | No |
| Authentication management | Yes | No | No |
| Role-based testing | Yes | No | No |
| Entity dependency resolution | Yes | Partial | No |
| Symbolic execution | Yes | No | No |
| Constraint solving | Yes | No | No |
| Infeasibility detection | Yes | No | No |
| Schema-based fuzzing | No | Yes | Yes |

## Troubleshooting

**Z3 not found:**
```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

**Connection refused:**
Ensure the target backend is running on the correct port before using `FULL_PIPELINE` mode.

**UNSAT result:**
The test sequence is infeasible given the current specification preconditions. This is expected behaviour — TestGen reports it and moves on.

## Acknowledgments

- [Z3 Theorem Prover](https://github.com/Z3Prover/z3) by Microsoft Research
- [nlohmann/json](https://github.com/nlohmann/json) for JSON parsing
- [libcurl](https://curl.se/libcurl/) for HTTP requests
