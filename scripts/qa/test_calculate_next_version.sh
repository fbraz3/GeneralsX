#!/usr/bin/env bash
# Automated test suite for calculate_next_version.sh
#
# Usage:
#   ./scripts/qa/test_calculate_next_version.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CALCULATE_SCRIPT="${SCRIPT_DIR}/../tooling/release/calculate_next_version.sh"

TEST_DIR="$(mktemp -d -t test_calculate_next_version_XXXXXX)"
trap 'rm -rf "$TEST_DIR"' EXIT

pass_count=0
fail_count=0

assert_equals() {
  local test_name="$1"
  local expected="$2"
  local actual="$3"

  if [ "$expected" = "$actual" ]; then
    echo "  [PASS] $test_name -> $actual"
    pass_count=$((pass_count + 1))
  else
    echo "  [FAIL] $test_name -> expected '$expected', got '$actual'" >&2
    fail_count=$((fail_count + 1))
  fi
}

echo "Running tests for calculate_next_version.sh..."

# Test 1: Empty repo with base-version 1.0 -> 1.0.0
REPO1="${TEST_DIR}/repo1"
mkdir -p "$REPO1"
git -C "$REPO1" init -q -b main
git -C "$REPO1" config user.name "Test"
git -C "$REPO1" config user.email "test@test.local"
echo "1.0" > "${REPO1}/base-version.txt"
git -C "$REPO1" add base-version.txt
git -C "$REPO1" commit -q -m "initial"
OUTPUT1="$(cd "$REPO1" && "$CALCULATE_SCRIPT")"
assert_equals "No prior tags with 1.0" "1.0.0" "$OUTPUT1"

# Test 2: Tag 1.0.0 exists -> 1.0.1
git -C "$REPO1" tag "1.0.0"
OUTPUT2="$(cd "$REPO1" && "$CALCULATE_SCRIPT")"
assert_equals "Tag 1.0.0 exists" "1.0.1" "$OUTPUT2"

# Test 3: Numeric sorting (1.0.9 vs 1.0.10)
git -C "$REPO1" tag "1.0.9"
git -C "$REPO1" tag "1.0.10"
OUTPUT3="$(cd "$REPO1" && "$CALCULATE_SCRIPT")"
assert_equals "Numeric sorting with 1.0.10" "1.0.11" "$OUTPUT3"

# Test 4: Base version changed to 1.5 -> 1.5.0
echo "1.5" > "${REPO1}/base-version.txt"
OUTPUT4="$(cd "$REPO1" && "$CALCULATE_SCRIPT")"
assert_equals "Base version 1.5 with no 1.5 tags" "1.5.0" "$OUTPUT4"

# Test 5: Tag v1.5.0 exists -> 1.5.1
git -C "$REPO1" tag "v1.5.0"
OUTPUT5="$(cd "$REPO1" && "$CALCULATE_SCRIPT")"
assert_equals "Tag v1.5.0 exists" "1.5.1" "$OUTPUT5"

# Test 6: Legacy prefix GeneralsX-1.5.3 exists -> 1.5.4
git -C "$REPO1" tag "GeneralsX-1.5.3"
OUTPUT6="$(cd "$REPO1" && "$CALCULATE_SCRIPT")"
assert_equals "Legacy prefix GeneralsX-1.5.3 exists" "1.5.4" "$OUTPUT6"

# Test 7: Whitespace trimming in base-version.txt
echo -e "  2.0 \n " > "${REPO1}/base-version.txt"
OUTPUT7="$(cd "$REPO1" && "$CALCULATE_SCRIPT")"
assert_equals "Whitespace handling for 2.0" "2.0.0" "$OUTPUT7"

# Test 8: GITHUB_OUTPUT writing
GH_OUT_FILE="${TEST_DIR}/github_output.txt"
touch "$GH_OUT_FILE"
GITHUB_OUTPUT="$GH_OUT_FILE" "$CALCULATE_SCRIPT" "${REPO1}/base-version.txt" >/dev/null
WRITTEN_CONTENT="$(cat "$GH_OUT_FILE")"
assert_equals "GITHUB_OUTPUT contains key-value" "version=2.0.0" "$WRITTEN_CONTENT"

# Test 9: Invalid version format error handling
echo "invalid_version" > "${REPO1}/base-version.txt"
if cd "$REPO1" && "$CALCULATE_SCRIPT" >/dev/null 2>&1; then
  echo "  [FAIL] Invalid format should fail" >&2
  fail_count=$((fail_count + 1))
else
  echo "  [PASS] Invalid format failed as expected"
  pass_count=$((pass_count + 1))
fi

echo "=========================================="
echo "Tests completed: $pass_count passed, $fail_count failed"
echo "=========================================="

if [ "$fail_count" -gt 0 ]; then
  exit 1
fi
