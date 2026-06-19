#!/usr/bin/env bash
# ============================================================================
#  TestGen Mutation Testing — Automation Script
# ============================================================================
#  This script applies each of the 54 mutations to the GhostSocket backend,
#  restarts the backend, runs the TestGen test suite, then reports whether
#  each mutant was killed (detected) or survived.
#
#  Prerequisites:
#    1. GhostSocket backend dependencies installed (cd GhostSocket/server && npm install)
#    2. MongoDB running and configured in GhostSocket/.env
#    3. TestGen binary compiled  (cd TestgenTool && make)
#    4. Python ≥3.8 available
#
#  Usage:
#    cd TestgenTool/mutation_testing
#    bash run_mutations.sh
# ============================================================================

set -euo pipefail

# ── Paths ──────────────────────────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TESTGEN_ROOT="$(dirname "$SCRIPT_DIR")"
BACKEND_ROOT="/Users/nakulpanwar/Desktop/GhostSocket/server"
TESTGEN_BIN="$TESTGEN_ROOT/test_libapplication"
MUTANTS_DIR="$SCRIPT_DIR/mutants"
RESULTS_FILE="$SCRIPT_DIR/mutation_results.txt"
LOG_DIR="$SCRIPT_DIR/logs"

mkdir -p "$MUTANTS_DIR" "$LOG_DIR"

# ── Colour helpers ──────────────────────────────────────────────────────────
GREEN='\033[0;32m'; RED='\033[0;31m'; YELLOW='\033[1;33m'; NC='\033[0m'

# ── Helper: start GhostSocket backend ──────────────────────────────────────
BACKEND_PID=""

start_backend() {
    echo -e "${YELLOW}[backend] Starting GhostSocket…${NC}"
    cd "$BACKEND_ROOT"
    node index.js &> "$LOG_DIR/backend.log" &
    BACKEND_PID=$!
    sleep 3   # wait for startup
    echo -e "${GREEN}[backend] PID=$BACKEND_PID${NC}"
    cd "$SCRIPT_DIR"
}

stop_backend() {
    if [[ -n "$BACKEND_PID" ]]; then
        kill "$BACKEND_PID" 2>/dev/null || true
        wait "$BACKEND_PID" 2>/dev/null || true
        BACKEND_PID=""
        sleep 1
    fi
}

# ── Helper: apply one mutation ───────────────────────────────────────────────
apply_mutation() {
    local MUT_ID="$1"
    local SRC_FILE="$2"
    local MUTANT_FILE="$MUTANTS_DIR/$MUT_ID/$(basename "$SRC_FILE")"
    if [[ ! -f "$MUTANT_FILE" ]]; then
        echo "  [SKIP] $MUT_ID — mutant file not found (run: python3 run_mutation_testing.py --apply)"
        return 1
    fi
    cp "$SRC_FILE" "${SRC_FILE}.orig"   # backup original
    cp "$MUTANT_FILE" "$SRC_FILE"        # apply mutant
}

restore_original() {
    local SRC_FILE="$1"
    if [[ -f "${SRC_FILE}.orig" ]]; then
        cp "${SRC_FILE}.orig" "$SRC_FILE"
        rm "${SRC_FILE}.orig"
    fi
}

# ── Helper: run TestGen and decide kill/survive ─────────────────────────────
run_testgen() {
    local LOG="$1"
    "$TESTGEN_BIN" ghostsocket >"$LOG" 2>&1 || true
    # Killed = any "FAILED" or "✗" or non-zero return
    if grep -qE "(✗|FAILED|Error:|assertion)" "$LOG" 2>/dev/null; then
        echo "KILLED"
    else
        echo "SURVIVED"
    fi
}

# ── Step 1: generate mutant files ───────────────────────────────────────────
echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║      TestGen Mutation Testing — GhostSocket Backend      ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "[Step 1] Generating mutant files…"
python3 "$SCRIPT_DIR/run_mutation_testing.py" --apply
echo ""

# ── Step 2: capture baseline run ────────────────────────────────────────────
echo "[Step 2] Capturing baseline TestGen output…"
start_backend
"$TESTGEN_BIN" ghostsocket > "$LOG_DIR/baseline.log" 2>&1 || true
stop_backend
echo "  Baseline saved to $LOG_DIR/baseline.log"
echo ""

# ── Step 3: run TestGen against each mutant ─────────────────────────────────
echo "[Step 3] Running TestGen against all 54 mutants…"
echo ""
printf "%-6s %-8s %-55s %s\n" "ID" "OPERATOR" "DESCRIPTION" "RESULT"
printf '%s\n' "$(printf '─%.0s' {1..90})"

# Mutation catalogue (id | operator | file | description)
declare -A MUTANT_FILES=(
    [M01]="controllers/SessionController.js"
    [M02]="controllers/SessionController.js"
    [M03]="controllers/SessionController.js"
    [M04]="controllers/SessionController.js"
    [M05]="controllers/SessionController.js"
    [M06]="controllers/SessionController.js"
    [M07]="controllers/SessionController.js"
    [M08]="controllers/SessionController.js"
    [M09]="controllers/SessionController.js"
    [M10]="controllers/SessionController.js"
    [M11]="controllers/SessionController.js"
    [M12]="controllers/SessionController.js"
    [M13]="controllers/SessionController.js"
    [M14]="controllers/SessionController.js"
    [M15]="controllers/SessionController.js"
    [M16]="controllers/SessionController.js"
    [M17]="controllers/SessionController.js"
    [M18]="controllers/SessionController.js"
    [M19]="controllers/SessionController.js"
    [M20]="controllers/SessionController.js"
    [M21]="controllers/SessionController.js"
    [M22]="controllers/SessionController.js"
    [M23]="controllers/SessionController.js"
    [M24]="controllers/SessionController.js"
    [M25]="controllers/SessionController.js"
    [M26]="controllers/SessionController.js"
    [M27]="controllers/SessionController.js"
    [M28]="controllers/DeviceController.js"
    [M29]="controllers/DeviceController.js"
    [M30]="controllers/DeviceController.js"
    [M31]="controllers/DeviceController.js"
    [M32]="controllers/DeviceController.js"
    [M33]="controllers/DeviceController.js"
    [M34]="controllers/DeviceController.js"
    [M35]="controllers/DeviceController.js"
    [M36]="controllers/DeviceController.js"
    [M37]="controllers/DeviceController.js"
    [M38]="controllers/DeviceController.js"
    [M39]="controllers/DeviceController.js"
    [M40]="controllers/DeviceController.js"
    [M41]="controllers/DeviceController.js"
    [M42]="controllers/DeviceController.js"
    [M43]="controllers/AppController.js"
    [M44]="controllers/AppController.js"
    [M45]="controllers/AppController.js"
    [M46]="controllers/AppController.js"
    [M47]="controllers/AppController.js"
    [M48]="controllers/AppController.js"
    [M49]="controllers/AppController.js"
    [M50]="controllers/AppController.js"
    [M51]="controllers/AppController.js"
    [M52]="controllers/AppController.js"
    [M53]="controllers/AppController.js"
    [M54]="controllers/AppController.js"
)

declare -A OPERATORS=(
    [M01]=CDL [M02]=CDL [M03]=UOI [M04]=CDL [M05]=ROR [M06]=ROR [M07]=AOR [M08]=AOR
    [M09]=ROR [M10]=LCR [M11]=SDL [M12]=CDL [M13]=CDL [M14]=SDL [M15]=CDL [M16]=ROR
    [M17]=SDL [M18]=SDL [M19]=ROR [M20]=UOI [M21]=CDL [M22]=SDL [M23]=CDL [M24]=LCR
    [M25]=CDL [M26]=SDL [M27]=CDL [M28]=UOI [M29]=CDL [M30]=SDL [M31]=CDL [M32]=CDL
    [M33]=ROR [M34]=CDL [M35]=SDL [M36]=SDL [M37]=SDL [M38]=CDL [M39]=CDL [M40]=CDL
    [M41]=CDL [M42]=CDL [M43]=ROR [M44]=ROR [M45]=SDL [M46]=SDL [M47]=CDL [M48]=CDL
    [M49]=CDL [M50]=SDL [M51]=SDL [M52]=AOR [M53]=AOR [M54]=SDL
)

KILLED=0
SURVIVED=0
SKIPPED=0
> "$RESULTS_FILE"
echo "TestGen Mutation Testing Results" >> "$RESULTS_FILE"
echo "Generated: $(date)" >> "$RESULTS_FILE"
echo "" >> "$RESULTS_FILE"

for MUT_ID in $(echo "${!MUTANT_FILES[@]}" | tr ' ' '\n' | sort); do
    REL_FILE="${MUTANT_FILES[$MUT_ID]}"
    ABS_FILE="$BACKEND_ROOT/$REL_FILE"
    OP="${OPERATORS[$MUT_ID]}"
    MUTANT_FILE="$MUTANTS_DIR/$MUT_ID/$(basename "$REL_FILE")"

    if [[ ! -f "$MUTANT_FILE" ]]; then
        printf "%-6s %-8s %-55s %s\n" "$MUT_ID" "$OP" "(mutant file missing)" "SKIP"
        echo "$MUT_ID SKIPPED" >> "$RESULTS_FILE"
        ((SKIPPED++)) || true
        continue
    fi

    # Apply mutation + start backend
    cp "$ABS_FILE" "${ABS_FILE}.orig"
    cp "$MUTANT_FILE" "$ABS_FILE"
    start_backend

    # Run TestGen
    RESULT=$(run_testgen "$LOG_DIR/${MUT_ID}.log")

    # Restore + stop backend
    cp "${ABS_FILE}.orig" "$ABS_FILE"
    rm "${ABS_FILE}.orig"
    stop_backend

    # Report
    DESC=$(cat "$MUTANTS_DIR/$MUT_ID/meta.txt" 2>/dev/null | grep "Description" | cut -d: -f2- | xargs | cut -c1-55)
    if [[ "$RESULT" == "KILLED" ]]; then
        printf "${GREEN}%-6s %-8s %-55s KILLED   ✓${NC}\n" "$MUT_ID" "$OP" "$DESC"
        ((KILLED++)) || true
    else
        printf "${RED}%-6s %-8s %-55s SURVIVED ✗${NC}\n" "$MUT_ID" "$OP" "$DESC"
        ((SURVIVED++)) || true
    fi
    echo "$MUT_ID $RESULT $OP $DESC" >> "$RESULTS_FILE"
done

TOTAL=$((KILLED + SURVIVED + SKIPPED))
SCORE=$(echo "scale=1; $KILLED * 100 / ($KILLED + $SURVIVED)" | bc 2>/dev/null || echo "N/A")

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
printf "║  MUTATION SCORE: %3d / %3d killed = %5s%%               ║\n" \
    "$KILLED" "$((KILLED+SURVIVED))" "$SCORE"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Full results saved to: $RESULTS_FILE"

echo "" >> "$RESULTS_FILE"
echo "TOTAL: $KILLED killed / $((KILLED+SURVIVED)) tested = $SCORE% mutation score" >> "$RESULTS_FILE"
