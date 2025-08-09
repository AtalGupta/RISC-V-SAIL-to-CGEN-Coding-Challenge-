/**
 * @file parser.c
 * @brief JSON lexical analysis and parsing implementation
 * 
 * This module provides a complete JSON parser with tokenization,
 * recursive descent parsing, and AST construction.
 */

#include "json_to_sexpr.h"

/**
 * @brief Initializes the parser with input text and prepares for parsing
 * @param parser The parser context to initialize
 * @param input The JSON input string to parse
 */
void parser_initialize(parser_t *parser, const char *input) {
    parser->input = input;
    parser->pos = 0;
    parser->length = strlen(input);
    parser->line = 1;
    parser->column = 1;
    parser->current_token = tokenizer_get_next_token(parser);
}

/**
 * @brief Skips whitespace characters and updates position tracking
 * @param parser The parser context
 */
void tokenizer_skip_whitespace(parser_t *parser) {
    while (parser->pos < parser->length) {
        const char current_char = parser->input[parser->pos];
        
        if (current_char == ' ' || current_char == '\t' || current_char == '\r') {
            parser->pos++;
            parser->column++;
        } else if (current_char == '\n') {
            parser->pos++;
            parser->line++;
            parser->column = 1;
        } else {
            break;
        }
    }
}

/**
 * @brief Parses a JSON string literal token with escape sequence handling
 * @param parser The parser context
 * @return A token containing the parsed string value
 */
token_t tokenizer_parse_string_literal(parser_t *parser) {
    token_t token = {TOKEN_STRING, "", 0.0};
    size_t value_pos = 0;
    bool found_closing_quote = false;
    
    parser->pos++; // Skip opening quote
    parser->column++;
    
    while (parser->pos < parser->length && value_pos < MAX_TOKEN_SIZE - 1) {
        char c = parser->input[parser->pos];
        
        if (c == '"') {
            parser->pos++;
            parser->column++;
            found_closing_quote = true;
            break;
        } else if (c == '\\' && parser->pos + 1 < parser->length) {
            parser->pos++;
            parser->column++;
            char escaped = parser->input[parser->pos];
            switch (escaped) {
                case '"': token.value[value_pos++] = '"'; break;
                case '\\': token.value[value_pos++] = '\\'; break;
                case '/': token.value[value_pos++] = '/'; break;
                case 'b': token.value[value_pos++] = '\b'; break;
                case 'f': token.value[value_pos++] = '\f'; break;
                case 'n': token.value[value_pos++] = '\n'; break;
                case 'r': token.value[value_pos++] = '\r'; break;
                case 't': token.value[value_pos++] = '\t'; break;
                default: 
                    token.value[value_pos++] = '\\';
                    if (value_pos < MAX_TOKEN_SIZE - 1) {
                        token.value[value_pos++] = escaped;
                    }
                    break;
            }
            parser->pos++;
            parser->column++;
        } else {
            token.value[value_pos++] = c;
            parser->pos++;
            parser->column++;
        }
    }
    
    // Check if string was properly closed
    if (!found_closing_quote) {
        fprintf(stderr, "Unterminated string at line %d, column %d\n", 
                parser->line, parser->column);
        token.type = TOKEN_ERROR;
        return token;
    }
    
    token.value[value_pos] = '\0';
    return token;
}

/* Parse a JSON number token */
token_t tokenizer_parse_numeric_literal(parser_t *parser) {
    token_t token = {TOKEN_NUMBER, "", 0.0};
    size_t value_pos = 0;
    
    // Handle negative numbers
    if (parser->input[parser->pos] == '-') {
        token.value[value_pos++] = '-';
        parser->pos++;
        parser->column++;
    }
    
    // Check for invalid leading zero pattern (like "01", "02", etc.)
    if (parser->pos < parser->length && parser->input[parser->pos] == '0') {
        token.value[value_pos++] = '0';
        parser->pos++;
        parser->column++;
        
        // If next character is a digit, this is invalid (leading zero)
        if (parser->pos < parser->length && isdigit(parser->input[parser->pos])) {
            fprintf(stderr, "Invalid number with leading zero at line %d, column %d\n", 
                    parser->line, parser->column);
            token.type = TOKEN_ERROR;
            return token;
        }
    } else {
        // Parse integer part (non-zero start)
        while (parser->pos < parser->length && isdigit(parser->input[parser->pos])) {
            if (value_pos < MAX_TOKEN_SIZE - 1) {
                token.value[value_pos++] = parser->input[parser->pos];
            }
            parser->pos++;
            parser->column++;
        }
    }
    
    // Parse fractional part
    if (parser->pos < parser->length && parser->input[parser->pos] == '.') {
        if (value_pos < MAX_TOKEN_SIZE - 1) {
            token.value[value_pos++] = '.';
        }
        parser->pos++;
        parser->column++;
        
        while (parser->pos < parser->length && isdigit(parser->input[parser->pos])) {
            if (value_pos < MAX_TOKEN_SIZE - 1) {
                token.value[value_pos++] = parser->input[parser->pos];
            }
            parser->pos++;
            parser->column++;
        }
    }
    
    // Parse exponent part
    if (parser->pos < parser->length && 
        (parser->input[parser->pos] == 'e' || parser->input[parser->pos] == 'E')) {
        if (value_pos < MAX_TOKEN_SIZE - 1) {
            token.value[value_pos++] = parser->input[parser->pos];
        }
        parser->pos++;
        parser->column++;
        
        if (parser->pos < parser->length && 
            (parser->input[parser->pos] == '+' || parser->input[parser->pos] == '-')) {
            if (value_pos < MAX_TOKEN_SIZE - 1) {
                token.value[value_pos++] = parser->input[parser->pos];
            }
            parser->pos++;
            parser->column++;
        }
        
        while (parser->pos < parser->length && isdigit(parser->input[parser->pos])) {
            if (value_pos < MAX_TOKEN_SIZE - 1) {
                token.value[value_pos++] = parser->input[parser->pos];
            }
            parser->pos++;
            parser->column++;
        }
    }
    
    token.value[value_pos] = '\0';
    token.number_value = atof(token.value);
    return token;
}

/* Parse JSON keywords (true, false, null) */
token_t tokenizer_parse_keyword_literal(parser_t *parser) {
    token_t token = {TOKEN_ERROR, "", 0.0};
    
    if (strncmp(parser->input + parser->pos, "true", 4) == 0) {
        token.type = TOKEN_TRUE;
        strcpy(token.value, "true");
        parser->pos += 4;
        parser->column += 4;
    } else if (strncmp(parser->input + parser->pos, "false", 5) == 0) {
        token.type = TOKEN_FALSE;
        strcpy(token.value, "false");
        parser->pos += 5;
        parser->column += 5;
    } else if (strncmp(parser->input + parser->pos, "null", 4) == 0) {
        token.type = TOKEN_NULL;
        strcpy(token.value, "null");
        parser->pos += 4;
        parser->column += 4;
    }
    
    return token;
}

/* Get next token from input */
token_t tokenizer_get_next_token(parser_t *parser) {
    tokenizer_skip_whitespace(parser);
    
    if (parser->pos >= parser->length) {
        token_t token = {TOKEN_EOF, "", 0.0};
        return token;
    }
    
    char c = parser->input[parser->pos];
    token_t token = {TOKEN_ERROR, "", 0.0};
    
    switch (c) {
        case '{':
            token.type = TOKEN_LBRACE;
            token.value[0] = c;
            token.value[1] = '\0';
            parser->pos++;
            parser->column++;
            break;
        case '}':
            token.type = TOKEN_RBRACE;
            token.value[0] = c;
            token.value[1] = '\0';
            parser->pos++;
            parser->column++;
            break;
        case '[':
            token.type = TOKEN_LBRACKET;
            token.value[0] = c;
            token.value[1] = '\0';
            parser->pos++;
            parser->column++;
            break;
        case ']':
            token.type = TOKEN_RBRACKET;
            token.value[0] = c;
            token.value[1] = '\0';
            parser->pos++;
            parser->column++;
            break;
        case ':':
            token.type = TOKEN_COLON;
            token.value[0] = c;
            token.value[1] = '\0';
            parser->pos++;
            parser->column++;
            break;
        case ',':
            token.type = TOKEN_COMMA;
            token.value[0] = c;
            token.value[1] = '\0';
            parser->pos++;
            parser->column++;
            break;
        case '"':
            token = tokenizer_parse_string_literal(parser);
            break;
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            token = tokenizer_parse_numeric_literal(parser);
            break;
        case 't': case 'f': case 'n':
            token = tokenizer_parse_keyword_literal(parser);
            break;
        default:
            fprintf(stderr, "Unexpected character '%c' at line %d, column %d\n", 
                    c, parser->line, parser->column);
            break;
    }
    
    return token;
}

/* Parse JSON value */
json_value_t *json_parser_parse_value(parser_t *parser) {
    json_value_t *value = malloc(sizeof(json_value_t));
    if (!value) return NULL;
    
    switch (parser->current_token.type) {
        case TOKEN_LBRACE:
            free(value);
            return json_parser_parse_object(parser);
        case TOKEN_LBRACKET:
            free(value);
            return json_parser_parse_array(parser);
        case TOKEN_STRING:
            value->type = JSON_STRING;
            value->data.string = malloc(strlen(parser->current_token.value) + 1);
            if (value->data.string) {
                strcpy(value->data.string, parser->current_token.value);
            }
            parser->current_token = tokenizer_get_next_token(parser);
            break;
        case TOKEN_NUMBER:
            value->type = JSON_NUMBER;
            value->data.number = parser->current_token.number_value;
            parser->current_token = tokenizer_get_next_token(parser);
            break;
        case TOKEN_TRUE:
            value->type = JSON_BOOLEAN;
            value->data.boolean = true;
            parser->current_token = tokenizer_get_next_token(parser);
            break;
        case TOKEN_FALSE:
            value->type = JSON_BOOLEAN;
            value->data.boolean = false;
            parser->current_token = tokenizer_get_next_token(parser);
            break;
        case TOKEN_NULL:
            value->type = JSON_NULL;
            parser->current_token = tokenizer_get_next_token(parser);
            break;
        case TOKEN_ERROR:
            fprintf(stderr, "Parse error: Invalid token encountered\n");
            free(value);
            return NULL;
        default:
            fprintf(stderr, "Parse error: Unexpected token type\n");
            free(value);
            return NULL;
    }
    
    return value;
}

/* Parse JSON object */
json_value_t *json_parser_parse_object(parser_t *parser) {
    json_value_t *object = malloc(sizeof(json_value_t));
    if (!object) return NULL;
    
    object->type = JSON_OBJECT;
    object->data.object = NULL;
    
    parser->current_token = tokenizer_get_next_token(parser); // Skip '{'
    
    if (parser->current_token.type == TOKEN_RBRACE) {
        parser->current_token = tokenizer_get_next_token(parser); // Skip '}'
        return object;
    }
    
    json_member_t *last_member = NULL;
    
    while (parser->current_token.type != TOKEN_EOF) {
        if (parser->current_token.type != TOKEN_STRING) {
            fprintf(stderr, "Expected string key in object\n");
            json_memory_free_value(object);
            return NULL;
        }
        
        json_member_t *member = malloc(sizeof(json_member_t));
        if (!member) {
            json_memory_free_value(object);
            return NULL;
        }
        
        member->key = malloc(strlen(parser->current_token.value) + 1);
        if (member->key) {
            strcpy(member->key, parser->current_token.value);
        }
        member->value = NULL;
        member->next = NULL;
        
        parser->current_token = tokenizer_get_next_token(parser); // Skip key
        
        if (parser->current_token.type != TOKEN_COLON) {
            fprintf(stderr, "Expected ':' after object key\n");
            free(member->key);
            free(member);
            json_memory_free_value(object);
            return NULL;
        }
        
        parser->current_token = tokenizer_get_next_token(parser); // Skip ':'
        
        member->value = json_parser_parse_value(parser);
        if (!member->value) {
            free(member->key);
            free(member);
            json_memory_free_value(object);
            return NULL;
        }
        
        if (object->data.object == NULL) {
            object->data.object = member;
        } else {
            last_member->next = member;
        }
        last_member = member;
        
        if (parser->current_token.type == TOKEN_COMMA) {
            parser->current_token = tokenizer_get_next_token(parser); // Skip ','
        } else if (parser->current_token.type == TOKEN_RBRACE) {
            parser->current_token = tokenizer_get_next_token(parser); // Skip '}'
            break;
        } else {
            fprintf(stderr, "Expected ',' or '}' in object\n");
            json_memory_free_value(object);
            return NULL;
        }
    }
    
    return object;
}

/* Parse JSON array */
json_value_t *json_parser_parse_array(parser_t *parser) {
    json_value_t *array = malloc(sizeof(json_value_t));
    if (!array) return NULL;
    
    array->type = JSON_ARRAY;
    array->data.array = NULL;
    
    parser->current_token = tokenizer_get_next_token(parser); // Skip '['
    
    if (parser->current_token.type == TOKEN_RBRACKET) {
        parser->current_token = tokenizer_get_next_token(parser); // Skip ']'
        return array;
    }
    
    json_element_t *last_element = NULL;
    
    while (parser->current_token.type != TOKEN_EOF) {
        json_element_t *element = malloc(sizeof(json_element_t));
        if (!element) {
            json_memory_free_value(array);
            return NULL;
        }
        
        element->value = json_parser_parse_value(parser);
        element->next = NULL;
        
        if (!element->value) {
            free(element);
            json_memory_free_value(array);
            return NULL;
        }
        
        if (array->data.array == NULL) {
            array->data.array = element;
        } else {
            last_element->next = element;
        }
        last_element = element;
        
        if (parser->current_token.type == TOKEN_COMMA) {
            parser->current_token = tokenizer_get_next_token(parser); // Skip ','
        } else if (parser->current_token.type == TOKEN_RBRACKET) {
            parser->current_token = tokenizer_get_next_token(parser); // Skip ']'
            break;
        } else {
            fprintf(stderr, "Expected ',' or ']' in array\n");
            json_memory_free_value(array);
            return NULL;
        }
    }
    
    return array;
}

/* Parse JSON from string */
json_value_t *json_parser_parse_document(parser_t *parser) {
    return json_parser_parse_value(parser);
}
