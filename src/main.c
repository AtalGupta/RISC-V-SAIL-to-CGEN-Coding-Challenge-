#include "json_to_sexpr.h"

/* Print usage information */
void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [OPTIONS] [INPUT_FILE]\n", program_name);
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "  -h, --help     Show this help message\n");
    fprintf(stderr, "  -o OUTPUT      Write output to file (default: stdout)\n");
    fprintf(stderr, "  -p, --pretty   Enable pretty printing with indentation\n");
    fprintf(stderr, "\nIf INPUT_FILE is not provided, reads from stdin.\n");
    fprintf(stderr, "\nExamples:\n");
    fprintf(stderr, "  %s input.json\n", program_name);
    fprintf(stderr, "  %s -o output.lisp input.json\n", program_name);
    fprintf(stderr, "  cat input.json | %s -p\n", program_name);
}

/* Read entire file into string */
char *read_file_to_string(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (size < 0) {
        perror("Error getting file size");
        fclose(file);
        return NULL;
    }
    
    // Allocate buffer
    char *buffer = malloc(size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(file);
        return NULL;
    }
    
    // Read file
    size_t bytes_read = fread(buffer, 1, size, file);
    buffer[bytes_read] = '\0';
    
    fclose(file);
    return buffer;
}

/* Read from stdin into string */
char *read_stdin_to_string(void) {
    size_t capacity = 1024;
    size_t size = 0;
    char *buffer = malloc(capacity);
    
    if (!buffer) {
        fprintf(stderr, "Error: Out of memory\n");
        return NULL;
    }
    
    int c;
    while ((c = getchar()) != EOF) {
        if (size >= capacity - 1) {
            capacity *= 2;
            char *new_buffer = realloc(buffer, capacity);
            if (!new_buffer) {
                fprintf(stderr, "Error: Out of memory\n");
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
        }
        buffer[size++] = c;
    }
    
    buffer[size] = '\0';
    return buffer;
}

int main(int argc, char *argv[]) {
    const char *input_filename = NULL;
    const char *output_filename = NULL;
    bool pretty_print = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--pretty") == 0) {
            pretty_print = true;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -o requires an output filename\n");
                print_usage(argv[0]);
                return 1;
            }
            output_filename = argv[++i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        } else {
            if (input_filename) {
                fprintf(stderr, "Error: Multiple input files specified\n");
                print_usage(argv[0]);
                return 1;
            }
            input_filename = argv[i];
        }
    }
    
    // Read input
    char *json_string;
    if (input_filename) {
        json_string = read_file_to_string(input_filename);
    } else {
        json_string = read_stdin_to_string();
    }
    
    if (!json_string) {
        return 1;
    }
    
    // Parse JSON
    parser_t parser;
    parser_initialize(&parser, json_string);
    
    json_value_t *json_value = json_parser_parse_document(&parser);
    if (!json_value) {
        fprintf(stderr, "Error: Failed to parse JSON\n");
        free(json_string);
        return 1;
    }
    
    // Check for remaining tokens (should be EOF)
    if (parser.current_token.type != TOKEN_EOF) {
        fprintf(stderr, "Warning: Extra content after JSON at line %d, column %d\n",
                parser.line, parser.column);
    }
    
    // Open output file
    FILE *output = stdout;
    if (output_filename) {
        output = fopen(output_filename, "w");
        if (!output) {
            perror("Error opening output file");
            json_memory_free_value(json_value);
            free(json_string);
            return 1;
        }
    }
    
    // Print S-expression
    fprintf(output, ";; JSON to S-expression conversion\n\n");
    
    sexpr_writer_write_value(json_value, output, 0);
    fprintf(output, "\n");
    
    // Cleanup
    if (output != stdout) {
        fclose(output);
    }
    
    json_memory_free_value(json_value);
    free(json_string);
    
    return 0;
}
