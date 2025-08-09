# SAIL to CGEN - Coding Challenge Solution

## JSON to S-Expression Converter

**Programming Language:** C   
**Input Format:** JSON   
**Output Format:** S-expressions   
**Author:** Atal Gupta
**Date:** August 9, 2025

## Challenge Overview

This program converts structured JSON data into S-expression representation, demonstrating adherence to well-defined transformation semantics as required by the SAIL to CGEN coding challenge.

## Quick Demo

```bash
# Build and run
make
echo '{"receipt": "Oz-Ware Purchase Invoice", "date": "2012-08-06"}' | ./json_to_sexpr
```

**Output:**
```lisp
;; JSON to S-expression conversion

(json:object
  (json:receipt "Oz-Ware Purchase Invoice")
  (json:date "2012-08-06"))
```

## Input Format Specification (JSON)

The program accepts valid JSON according to RFC 8259:
- **Objects**: `{"key": value, ...}`
- **Arrays**: `[element1, element2, ...]`
- **Strings**: `"text"` with escape sequences (`\"`, `\\`, `\n`, `\t`, `\uXXXX`)
- **Numbers**: integers, decimals, scientific notation
- **Booleans**: `true`, `false`
- **Null**: `null`

## Output Format Specification (S-expressions)

### Transformation Schema

| JSON Type | S-Expression Format | Example |
|-----------|-------------------|---------|
| Object | `(json:object (json:key value) ...)` | `{"a": 1}` → `(json:object (json:a 1))` |
| Array | `(json:array element1 element2 ...)` | `[1, 2]` → `(json:array 1 2)` |
| String | `"escaped-string"` | `"hello"` → `"hello"` |
| Number | Raw numeric value | `42`, `3.14`, `-5` |
| Boolean | `#t` or `#f` | `true` → `#t`, `false` → `#f` |
| Null | `nil` | `null` → `nil` |

### Namespace Convention
- `json:` prefix creates a namespace for JSON constructs
- Prevents naming conflicts with native Lisp symbols
- Makes output self-documenting and parseable

## Conversion Semantics

### 1. Structural Preservation
- Maintains exact nesting relationships from JSON
- Preserves order of object keys and array elements
- No data loss during transformation

### 2. Type Mapping
```c
// C implementation ensures consistent type conversion
JSON_STRING  → Lisp string with proper escaping
JSON_NUMBER  → Lisp number (integer or float)
JSON_BOOLEAN → Lisp boolean (#t/#f)
JSON_NULL    → Lisp nil
JSON_OBJECT  → Named list with json:object tag
JSON_ARRAY   → Named list with json:array tag
```

### 3. Escape Sequence Handling
- JSON `\"` → Lisp `\"`
- JSON `\\` → Lisp `\\`
- JSON `\n` → Lisp `\n`
- JSON `\uXXXX` → Preserved as `\uXXXX`

## Example Transformation

### Input JSON:
```json
{
  "receipt": "Oz-Ware Purchase Invoice",
  "date": "2012-08-06",
  "customer": {
    "first_name": "Dorothy",
    "family_name": "Gale"
  },
  "items": [
    {
      "part_no": "A4786",
      "descrip": "Water Bucket (Filled)",
      "price": 1.47,
      "quantity": 4
    },
    {
      "part_no": "E1628",
      "descrip": "High Heeled \"Ruby\" Slippers", 
      "price": 133.7,
      "quantity": 1
    }
  ]
}
```

### Output S-Expression:
```lisp
;; JSON to S-expression conversion

(json:object
  (json:receipt "Oz-Ware Purchase Invoice")
  (json:date "2012-08-06")
  (json:customer (json:object
      (json:first_name "Dorothy")
      (json:family_name "Gale")))
  (json:items (json:array
      (json:object
        (json:part_no "A4786")
        (json:descrip "Water Bucket (Filled)")
        (json:price 1.47)
        (json:quantity 4))
      (json:object
        (json:part_no "E1628")
        (json:descrip "High Heeled \"Ruby\" Slippers")
        (json:price 133.7)
        (json:quantity 1)))))
```

## Build and Usage

### Prerequisites
- GCC compiler with C99 support
- GNU Make

### Build
```bash
make                    # Optimized build
make debug             # Debug build with symbols
```

### Usage
```bash
./json_to_sexpr [input.json]           # File input
echo '{"test": 123}' | ./json_to_sexpr # Stdin input
./json_to_sexpr -o output.lisp input.json  # File output
./json_to_sexpr --help                 # Help message
```


### Key Design Decisions

1. **Recursive Descent Parser**: Handles arbitrary nesting depth
2. **AST Representation**: In-memory tree preserves structure
3. **Namespace Prefixing**: `json:` prevents symbol conflicts
4. **Memory Safety**: Zero leaks, proper cleanup on errors
5. **Error Reporting**: Line/column precision for debugging

## Validation and Testing



### Example Test Cases
```bash
# Valid transformations
echo '{}' | ./json_to_sexpr                    # Empty object
echo '[]' | ./json_to_sexpr                    # Empty array  
echo '"hello\nworld"' | ./json_to_sexpr        # Escaped strings
echo '1.23e-4' | ./json_to_sexpr               # Scientific notation

# Error detection
echo '{"unclosed": ' | ./json_to_sexpr         # Syntax error
echo '01' | ./json_to_sexpr                    # Invalid number
```



**Files Included:**
- Source code (`src/`, `include/`)
- Build system (`Makefile`, `CMakeLists.txt`)
- Test suite (`scripts/aggressive_test.sh`)
- Sample data (`test.json`)
- Documentation (`README.md`)

