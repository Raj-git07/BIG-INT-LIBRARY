# ════════════════════════════════════════════════════════════════
#  BigInt Library — Makefile  (MinGW g++ on Windows)
# ════════════════════════════════════════════════════════════════
CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Wpedantic

SRC      = BigInt.cpp
DEMO_SRC = main.cpp
TEST_SRC = test.cpp

DEMO_BIN = bigint_demo.exe
TEST_BIN = bigint_test.exe

# ── Default target: build everything ──────────────────────────
all: $(DEMO_BIN) $(TEST_BIN)

# ── Demo binary ───────────────────────────────────────────────
$(DEMO_BIN): $(SRC) $(DEMO_SRC) BigInt.h
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) $(DEMO_SRC)
	@echo "[OK] Built $(DEMO_BIN)"

# ── Test binary ───────────────────────────────────────────────
$(TEST_BIN): $(SRC) $(TEST_SRC) BigInt.h
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) $(TEST_SRC)
	@echo "[OK] Built $(TEST_BIN)"

# ── Run the demo ──────────────────────────────────────────────
run: $(DEMO_BIN)
	./$(DEMO_BIN)

# ── Run the test suite ────────────────────────────────────────
test: $(TEST_BIN)
	./$(TEST_BIN)

# ── Build + run both ──────────────────────────────────────────
check: all test

# ── Clean ─────────────────────────────────────────────────────
clean:
	del /Q $(DEMO_BIN) $(TEST_BIN) 2>NUL || true

.PHONY: all run test check clean
