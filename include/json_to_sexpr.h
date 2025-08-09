#ifndef JSON_TO_SEXPR_H
#define JSON_TO_SEXPR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* Maximum buffer sizes */
#define MAX_TOKEN_SIZE 1024
#define MAX_STRING_SIZE 2048
#define MAX_DEPTH 64

/* Token types for JSON parsing */
typedef enum {
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE,       // }
    TOKEN_LBRACKET,     // [
    TOKEN_RBRACKET,     // ]
    TOKEN_COLON,        // :
    TOKEN_COMMA,        // ,
    TOKEN_STRING,       // "string"
    TOKEN_NUMBER,       // 123, 123.45
    TOKEN_TRUE,         // true
    TOKEN_FALSE,        // false
    TOKEN_NULL,         // null
    TOKEN_EOF,          // end of file
    TOKEN_ERROR         // error token
} token_type_t;

/* Token structure */
typedef struct {
    token_type_t type;
    char value[MAX_TOKEN_SIZE];
    double number_value;
} token_t;

/* JSON value types */
typedef enum {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOLEAN,
    JSON_NULL
} json_type_t;

/* Forward declaration */
struct json_value;

/* JSON object member */
typedef struct json_member {
    char *key;
    struct json_value *value;
    struct json_member *next;
} json_member_t;

/* JSON array element */
typedef struct json_element {
    struct json_value *value;
    struct json_element *next;
} json_element_t;

/* JSON value structure */
typedef struct json_value {
    json_type_t type;
    union {
        json_member_t *object;
        json_element_t *array;
        char *string;
        double number;
        bool boolean;
    } data;
} json_value_t;

/* Parser context */
typedef struct {
    const char *input;
    size_t pos;
    size_t length;
    token_t current_token;
    int line;
    int column;
} parser_t;

/* Parser initialization and tokenization */
void parser_initialize(parser_t *parser, const char *input);
token_t tokenizer_get_next_token(parser_t *parser);
void tokenizer_skip_whitespace(parser_t *parser);
token_t tokenizer_parse_string_literal(parser_t *parser);
token_t tokenizer_parse_numeric_literal(parser_t *parser);
token_t tokenizer_parse_keyword_literal(parser_t *parser);

/* JSON parsing functions */
json_value_t *json_parser_parse_document(parser_t *parser);
json_value_t *json_parser_parse_value(parser_t *parser);
json_value_t *json_parser_parse_object(parser_t *parser);
json_value_t *json_parser_parse_array(parser_t *parser);

/* S-expression output functions */
void sexpr_writer_write_value(json_value_t *value, FILE *output, int indentation_level);
void sexpr_writer_write_object_members(json_member_t *member, FILE *output, int indentation_level);
void sexpr_writer_write_array_elements(json_element_t *element, FILE *output, int indentation_level);

/* Memory management functions */
void json_memory_free_value(json_value_t *value);
void json_memory_free_object_member(json_member_t *member);
void json_memory_free_array_element(json_element_t *element);

/* String utility functions */
char *string_utils_escape_for_lisp(const char *input_string);
void output_formatter_write_indentation(FILE *output, int indentation_level);

#endif /* JSON_TO_SEXPR_H */
