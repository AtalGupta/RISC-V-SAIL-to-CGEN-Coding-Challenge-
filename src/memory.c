/**
 * @file memory.c
 * @brief Memory management for JSON AST structures
 * 
 * This module provides safe memory deallocation for all JSON data structures,
 * preventing memory leaks through recursive cleanup.
 */

#include "json_to_sexpr.h"

/**
 * @brief Recursively frees all members in a JSON object linked list
 * @param member_node The first member in the linked list to free
 */
void json_memory_free_object_member(json_member_t *member_node) {
    while (member_node != NULL) {
        json_member_t *next_member = member_node->next;
        
        free(member_node->key);
        json_memory_free_value(member_node->value);
        free(member_node);
        
        member_node = next_member;
    }
}

/**
 * @brief Recursively frees all elements in a JSON array linked list
 * @param element_node The first element in the linked list to free
 */
void json_memory_free_array_element(json_element_t *element_node) {
    while (element_node != NULL) {
        json_element_t *next_element = element_node->next;
        
        json_memory_free_value(element_node->value);
        free(element_node);
        
        element_node = next_element;
    }
}

/**
 * @brief Recursively frees a JSON value and all its contained data
 * @param json_value The JSON value to free (can be NULL)
 */
void json_memory_free_value(json_value_t *json_value) {
    if (json_value == NULL) {
        return;
    }
    
    switch (json_value->type) {
        case JSON_OBJECT:
            json_memory_free_object_member(json_value->data.object);
            break;
            
        case JSON_ARRAY:
            json_memory_free_array_element(json_value->data.array);
            break;
            
        case JSON_STRING:
            free(json_value->data.string);
            break;
            
        case JSON_NUMBER:
        case JSON_BOOLEAN:
        case JSON_NULL:
            // These types don't allocate additional memory
            break;
    }
    
    free(json_value);
}
