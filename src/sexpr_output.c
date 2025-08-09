/**
 * @file sexpr_output.c
 * @brief S-expression output formatting and generation
 * 
 * This module handles the conversion of JSON AST to properly formatted
 * S-expressions with appropriate escaping and indentation.
 */

#include "json_to_sexpr.h"

/**
 * @brief Escapes a string for safe output in S-expression format
 * @param input_string The string to escape
 * @return Newly allocated escaped string with quotes, or NULL on failure
 * @note Caller is responsible for freeing the returned string
 */
char *string_utils_escape_for_lisp(const char *input_string) {
    if (!input_string) {
        return NULL;
    }
    
    const size_t input_length = strlen(input_string);
    // Worst case: every character needs escaping plus surrounding quotes
    char *escaped_string = malloc(input_length * 2 + 3);
    if (!escaped_string) {
        return NULL;
    }
    
    size_t output_position = 0;
    escaped_string[output_position++] = '"';
    
    for (size_t input_position = 0; input_position < input_length; input_position++) {
        const char current_char = input_string[input_position];
        
        switch (current_char) {
            case '"':
                escaped_string[output_position++] = '\\';
                escaped_string[output_position++] = '"';
                break;
            case '\\':
                escaped_string[output_position++] = '\\';
                escaped_string[output_position++] = '\\';
                break;
            case '\n':
                escaped_string[output_position++] = '\\';
                escaped_string[output_position++] = 'n';
                break;
            case '\r':
                escaped_string[output_position++] = '\\';
                escaped_string[output_position++] = 'r';
                break;
            case '\t':
                escaped_string[output_position++] = '\\';
                escaped_string[output_position++] = 't';
                break;
            default:
                escaped_string[output_position++] = current_char;
                break;
        }
    }
    
    escaped_string[output_position++] = '"';
    escaped_string[output_position] = '\0';
    
    return escaped_string;
}

/**
 * @brief Writes indentation spaces for pretty-printing S-expressions
 * @param output The file stream to write to
 * @param indentation_level The depth level (each level = 2 spaces)
 */
void output_formatter_write_indentation(FILE *output, int indentation_level) {
    const int SPACES_PER_LEVEL = 2;
    const int total_spaces = indentation_level * SPACES_PER_LEVEL;
    
    for (int space_count = 0; space_count < total_spaces; space_count++) {
        fputc(' ', output);
    }
}

/**
 * @brief Writes JSON object members as S-expression format
 * @param member_node The first member in the linked list
 * @param output The file stream to write to
 * @param indentation_level Current indentation depth
 */
void sexpr_writer_write_object_members(json_member_t *member_node, FILE *output, int indentation_level) {
    bool is_first_member = true;
    
    while (member_node != NULL) {
        if (!is_first_member) {
            fprintf(output, "\n");
            output_formatter_write_indentation(output, indentation_level);
        }
        is_first_member = false;
        
        fprintf(output, "(json:%s ", member_node->key);
        sexpr_writer_write_value(member_node->value, output, indentation_level + 1);
        fprintf(output, ")");
        
        member_node = member_node->next;
    }
}

/**
 * @brief Writes JSON array elements as S-expression format
 * @param element_node The first element in the linked list
 * @param output The file stream to write to
 * @param indentation_level Current indentation depth
 */
void sexpr_writer_write_array_elements(json_element_t *element_node, FILE *output, int indentation_level) {
    bool is_first_element = true;
    
    while (element_node != NULL) {
        if (!is_first_element) {
            fprintf(output, "\n");
            output_formatter_write_indentation(output, indentation_level);
        }
        is_first_element = false;
        
        sexpr_writer_write_value(element_node->value, output, indentation_level);
        
        element_node = element_node->next;
    }
}

/**
 * @brief Converts a JSON value to S-expression format and writes to output
 * @param json_value The JSON value to convert
 * @param output The file stream to write to
 * @param indentation_level Current indentation depth for pretty printing
 */
void sexpr_writer_write_value(json_value_t *json_value, FILE *output, int indentation_level) {
    if (json_value == NULL) {
        fprintf(output, "nil");
        return;
    }
    
    switch (json_value->type) {
        case JSON_OBJECT:
            if (json_value->data.object != NULL) {
                fprintf(output, "(json:object\n");
                output_formatter_write_indentation(output, indentation_level + 1);
                sexpr_writer_write_object_members(json_value->data.object, output, indentation_level + 1);
                fprintf(output, ")");
            } else {
                fprintf(output, "(json:object)");
            }
            break;
            
        case JSON_ARRAY:
            if (json_value->data.array != NULL) {
                fprintf(output, "(json:array\n");
                output_formatter_write_indentation(output, indentation_level + 1);
                sexpr_writer_write_array_elements(json_value->data.array, output, indentation_level + 1);
                fprintf(output, ")");
            } else {
                fprintf(output, "(json:array)");
            }
            break;
            
        case JSON_STRING: {
            char *escaped_string = string_utils_escape_for_lisp(json_value->data.string);
            if (escaped_string != NULL) {
                fprintf(output, "%s", escaped_string);
                free(escaped_string);
            } else {
                fprintf(output, "\"\"");
            }
            break;
        }
        
        case JSON_NUMBER: {
            const double number_value = json_value->data.number;
            const long long integer_value = (long long)number_value;
            
            // Check if the number is effectively an integer
            if (number_value == (double)integer_value) {
                fprintf(output, "%lld", integer_value);
            } else {
                fprintf(output, "%.15g", number_value);
            }
            break;
        }
        
        case JSON_BOOLEAN:
            fprintf(output, "%s", json_value->data.boolean ? "#t" : "#f");
            break;
            
        case JSON_NULL:
            fprintf(output, "nil");
            break;
            
        default:
            fprintf(output, "nil"); // Fallback for unknown types
            break;
    }
}
