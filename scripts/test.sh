#!/bin/bash
# Comprehensive edge case testing for JSON to S-expression converter

PROG="./json_to_sexpr"
PASS=0
FAIL=0
TOTAL=0

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test function
run_test() {
    local name="$1"
    local input="$2"
    local expected_exit="$3"
    local description="$4"
    
    echo -e "${BLUE}TEST: $name${NC}"
    echo -e "  Description: $description"
    echo -e "  Input: $input"
    
    TOTAL=$((TOTAL + 1))
    
    if echo -e "$input" | $PROG > /dev/null 2>&1; then
        actual_exit=0
    else
        actual_exit=1
    fi
    
    if [ $actual_exit -eq $expected_exit ]; then
        echo -e "  ${GREEN}PASS${NC} (exit code: $actual_exit)"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}FAIL${NC} (expected: $expected_exit, got: $actual_exit)"
        FAIL=$((FAIL + 1))
    fi
    echo
}

# Test file function
run_file_test() {
    local name="$1"
    local filename="$2"
    local expected_exit="$3"
    local description="$4"
    
    echo -e "${BLUE}FILE TEST: $name${NC}"
    echo -e "  Description: $description"
    echo -e "  File: $filename"
    
    TOTAL=$((TOTAL + 1))
    
    if $PROG "$filename" > /dev/null 2>&1; then
        actual_exit=0
    else
        actual_exit=1
    fi
    
    if [ $actual_exit -eq $expected_exit ]; then
        echo -e "  ${GREEN}PASS${NC} (exit code: $actual_exit)"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}FAIL${NC} (expected: $expected_exit, got: $actual_exit)"
        FAIL=$((FAIL + 1))
    fi
    echo
}

echo -e "${YELLOW}=== JSON to S-Expression Converter - Aggressive Test Suite ===${NC}"
echo

# Build the program
echo -e "${BLUE}Building program...${NC}"
gcc -std=c99 -Wall -Wextra -pedantic -O2 -Iinclude src/*.c -o json_to_sexpr
if [ $? -ne 0 ]; then
    echo -e "${RED}BUILD FAILED${NC}"
    exit 1
fi
echo -e "${GREEN}Build successful${NC}"
echo

echo -e "${YELLOW}=== CATEGORY 1: Valid JSON Cases ===${NC}"
run_test "empty_object" '{}' 0 "Empty object should parse successfully"
run_test "empty_array" '[]' 0 "Empty array should parse successfully"
run_test "simple_number" '123' 0 "Simple number should parse"
run_test "simple_string" '"hello"' 0 "Simple string should parse"
run_test "boolean_true" 'true' 0 "Boolean true should parse"
run_test "boolean_false" 'false' 0 "Boolean false should parse"
run_test "null_value" 'null' 0 "Null value should parse"
run_test "nested_structure" '{"a":[1,2,{"b":true}]}' 0 "Nested object and array"

echo -e "${YELLOW}=== CATEGORY 2: Whitespace Handling ===${NC}"
run_test "leading_whitespace" '   {"a":1}' 0 "Leading whitespace tolerance"
run_test "trailing_whitespace" '{"a":1}   ' 0 "Trailing whitespace tolerance"
run_test "internal_whitespace" '{ "a" : 1 , "b" : 2 }' 0 "Internal whitespace tolerance"
run_test "newlines" '{\n"a":\n1\n}' 0 "Newline handling"
run_test "tabs" '{\t"a":\t1\t}' 0 "Tab handling"

echo -e "${YELLOW}=== CATEGORY 3: Number Edge Cases ===${NC}"
run_test "zero" '0' 0 "Zero should parse"
run_test "negative_zero" '-0' 0 "Negative zero should parse"
run_test "negative_number" '-123' 0 "Negative number should parse"
run_test "float_number" '123.456' 0 "Float number should parse"
run_test "scientific_notation" '1.23e+10' 0 "Scientific notation should parse"
run_test "scientific_negative" '1.23e-10' 0 "Negative scientific notation"
run_test "large_integer" '9007199254740991' 0 "Large integer within double precision"

echo -e "${YELLOW}=== CATEGORY 4: String Edge Cases ===${NC}"
run_test "empty_string" '""' 0 "Empty string should parse"
run_test "string_with_quotes" '"He said \"hello\""' 0 "String with escaped quotes"
run_test "string_with_backslash" '"C:\\\\path\\\\file"' 0 "String with backslashes"
run_test "string_with_newline" '"line1\\nline2"' 0 "String with newline escape"
run_test "string_with_tab" '"col1\\tcol2"' 0 "String with tab escape"

echo -e "${YELLOW}=== CATEGORY 5: Array Edge Cases ===${NC}"
run_test "single_element_array" '[1]' 0 "Array with single element"
run_test "mixed_type_array" '[1,"hello",true,null]' 0 "Array with mixed types"
run_test "nested_arrays" '[[1,2],[3,4]]' 0 "Nested arrays"

echo -e "${YELLOW}=== CATEGORY 6: Object Edge Cases ===${NC}"
run_test "single_member_object" '{"key":"value"}' 0 "Object with single member"
run_test "multiple_members" '{"a":1,"b":2,"c":3}' 0 "Object with multiple members"
run_test "nested_objects" '{"outer":{"inner":"value"}}' 0 "Nested objects"
run_test "duplicate_keys" '{"a":1,"a":2}' 0 "Duplicate keys (last wins semantically)"

echo -e "${YELLOW}=== CATEGORY 7: Invalid JSON - Syntax Errors ===${NC}"
run_test "trailing_comma_array" '[1,2,]' 1 "Trailing comma in array should fail"
run_test "trailing_comma_object" '{"a":1,}' 1 "Trailing comma in object should fail"
run_test "missing_comma_array" '[1 2]' 1 "Missing comma in array should fail"
run_test "missing_comma_object" '{"a":1 "b":2}' 1 "Missing comma in object should fail"
run_test "missing_colon" '{"a" 1}' 1 "Missing colon in object should fail"
run_test "missing_value" '{"a":}' 1 "Missing value in object should fail"
run_test "unclosed_array" '[1,2' 1 "Unclosed array should fail"
run_test "unclosed_object" '{"a":1' 1 "Unclosed object should fail"
run_test "unclosed_string" '"hello' 1 "Unclosed string should fail"
run_test "mismatched_brackets" '[}' 1 "Mismatched brackets should fail"

echo -e "${YELLOW}=== CATEGORY 8: Invalid Tokens ===${NC}"
run_test "invalid_keyword" 'tru' 1 "Invalid keyword should fail"
run_test "invalid_number" '01' 1 "Leading zero number should fail"
run_test "invalid_character" '@' 1 "Invalid character should fail"
run_test "lone_comma" ',' 1 "Lone comma should fail"
run_test "lone_colon" ':' 1 "Lone colon should fail"

echo -e "${YELLOW}=== CATEGORY 9: Empty/Invalid Input ===${NC}"
run_test "empty_input" '' 1 "Empty input should fail"
run_test "whitespace_only" '   ' 1 "Whitespace-only input should fail"

echo -e "${YELLOW}=== CATEGORY 10: Multiple Values ===${NC}"
run_test "multiple_values" '1 2' 0 "Multiple values (warns but processes first)"
run_test "multiple_objects" '{} []' 0 "Multiple objects (warns but processes first)"

echo -e "${YELLOW}=== CATEGORY 11: File Tests ===${NC}"
if [ -f "tests/data/test.json" ]; then
    run_file_test "valid_file" "tests/data/test.json" 0 "Valid JSON file should parse"
fi
run_file_test "nonexistent_file" "nonexistent.json" 1 "Nonexistent file should fail"

echo -e "${YELLOW}=== CATEGORY 12: Deep Nesting Test ===${NC}"
# Create deep nesting test
python3 - << 'EOF' > deep_test.json 2>/dev/null || python - << 'EOF' > deep_test.json
import json
# Create deeply nested structure
data = 0
for i in range(50):
    data = [data]
print(json.dumps(data))
EOF

if [ -f "deep_test.json" ]; then
    run_file_test "deep_nesting" "deep_test.json" 0 "Deep nesting should handle gracefully"
    rm -f deep_test.json
fi

echo -e "${YELLOW}=== CATEGORY 13: Large String Test ===${NC}"
# Test buffer limits
python3 - << 'EOF' > large_string_test.json 2>/dev/null || python - << 'EOF' > large_string_test.json
import json
# Create string near buffer limit (MAX_TOKEN_SIZE = 1024)
large_string = "a" * 1000
data = {"large": large_string}
print(json.dumps(data))
EOF

if [ -f "large_string_test.json" ]; then
    run_file_test "large_string" "large_string_test.json" 0 "Large string should handle gracefully"
    rm -f large_string_test.json
fi

echo -e "${YELLOW}=== CATEGORY 14: Command Line Interface Tests ===${NC}"
echo -e "${BLUE}CLI TEST: Help option${NC}"
if $PROG --help > /dev/null 2>&1; then
    echo -e "  ${GREEN}PASS${NC} (--help works)"
else
    echo -e "  ${RED}FAIL${NC} (--help failed)"
fi

echo -e "${BLUE}CLI TEST: Output to file${NC}"
echo '{"test":123}' | $PROG -o test_output.tmp
if [ -f "test_output.tmp" ] && grep -q "json:test" test_output.tmp; then
    echo -e "  ${GREEN}PASS${NC} (output to file works)"
    rm -f test_output.tmp
else
    echo -e "  ${RED}FAIL${NC} (output to file failed)"
fi

echo -e "${BLUE}CLI TEST: Invalid option${NC}"
if $PROG --invalid-option > /dev/null 2>&1; then
    echo -e "  ${RED}FAIL${NC} (should reject invalid option)"
else
    echo -e "  ${GREEN}PASS${NC} (rejects invalid option)"
fi

echo
echo -e "${YELLOW}=== TEST SUMMARY ===${NC}"
echo -e "Total tests: $TOTAL"
echo -e "${GREEN}Passed: $PASS${NC}"
echo -e "${RED}Failed: $FAIL${NC}"

if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}All tests passed! ${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed. Review the output above.${NC}"
    exit 1
fi
