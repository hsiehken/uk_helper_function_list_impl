#include "helper_function_list.h"

#include <stdio.h>
#include <stdlib.h>

struct HelperFunctionList *
helper_function_list_init(size_t capacity,
                          void (*m_entry_init)(HelperFunctionEntry *, const char *, const void *,
                                               const UK_EBPF_HELPER_RET_TYPE_t,
                                               const UK_EBPF_HELPER_ARG_TYPE_NUM_t, const UK_EBPF_HELPER_ARG_TYPE_t[]),
                          void (*entry_destructor)(HelperFunctionEntry *)) {
    HelperFunctionList *instance = malloc(sizeof(HelperFunctionList));

    if (instance == NULL) {
        return NULL;
    }

    instance->m_length = 0;
    instance->m_capacity = capacity;
    instance->m_entry_init = m_entry_init;
    instance->m_entry_destructor = entry_destructor;
    instance->m_entries = malloc(sizeof(HelperFunctionEntry) * capacity);

    if (instance->m_entries == NULL) {
        free(instance);
        return NULL;
    }

    return instance;
}

bool helper_function_list_resize(HelperFunctionList *self, size_t new_capacity) {
    if (self->m_capacity == new_capacity) {
        return true;
    }

    if (self->m_length > new_capacity) {
        // call de-ctor against entries that exceed the new capacity
        for (size_t index = new_capacity; index < self->m_length; index++) {
            self->m_entry_destructor(&self->m_entries[index]);
        }

        self->m_length = new_capacity;
    }

    HelperFunctionEntry *new_entry_buffer = realloc(self->m_entries, sizeof(HelperFunctionEntry) * new_capacity);
    if (new_entry_buffer == NULL && new_capacity > 0) {
        return false;
    }

    self->m_entries = new_entry_buffer;
    self->m_capacity = new_capacity;

    return true;
}

static size_t helper_find_greater_exponent(size_t target) {
    if (target == 0) {
        return 1;
    }

    if (target >> (sizeof(target) * 8 - 1)) {
        return 0; // failed, we can not find a greater exponent without overflow
    }

    const size_t result_lower_bound = target * 2;
    size_t result = result_lower_bound;

    while (result < target) {
        result *= 2;

        if (result <= result_lower_bound) {
            // overflow
            return 0;
        }
    }

    return result;
}

bool helper_function_list_push_back(HelperFunctionList *self,
                                    const char *functionName, const void *functionAddr, const UK_EBPF_HELPER_RET_TYPE_t retType,
                                    const UK_EBPF_HELPER_ARG_TYPE_NUM_t argTypeCount,
                                    const UK_EBPF_HELPER_ARG_TYPE_t argTypes[]) {
    if (self->m_length >= self->m_capacity) {
        // there is no more space in the buffer, we need to resize
        size_t new_capacity = helper_find_greater_exponent(self->m_capacity == 0 ? 1 : self->m_capacity);
        if (new_capacity == 0) {
            return false;
        }

        if (!helper_function_list_resize(self, new_capacity)) {
            return false;
        }
    }


    HelperFunctionEntry *entry = &self->m_entries[self->m_length];
    self->m_entry_init(entry, functionName, functionAddr, retType, argTypeCount, argTypes);
    self->m_length += 1;

    return true;
}

void helper_function_list_apply_function(HelperFunctionList *self, void (*apply)(struct HelperFunctionEntry *)) {
    for (size_t index = 0; index < self->m_length; index++) {
        apply(&self->m_entries[index]);
    }
}

void helper_function_list_remove_elem(HelperFunctionList *self, const char *label) {
    size_t removed_elements = 0;

    for (size_t index = 0; index < self->m_length; index++) {
        if (strncmp(self->m_entries[index].m_function_signature.m_function_name, label,
                    UK_EBPF_HELPER_FUNCTION_MAX_NAME_LENGTH) == 0) {
            self->m_entry_destructor(&self->m_entries[index]);
            removed_elements++;
        } else if (removed_elements > 0) {
            self->m_entries[index - removed_elements] = self->m_entries[index];
        }
    }

    self->m_length -= removed_elements;
}

void helper_function_list_print(HelperFunctionList *self, void (*printer)(struct HelperFunctionEntry *)) {
    helper_function_list_apply_function(self, printer);
}

void helper_function_list_destroy(HelperFunctionList *self) {
    helper_function_list_resize(self, 0);
    free(self->m_entries);
    free(self);
}