#ifndef HELPER_FUNCTION_LIST_H
#define HELPER_FUNCTION_LIST_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef enum UK_EBPF_HELPER_RET_TYPE {
    VOID = 0,
} __attribute__((__packed__)) UK_EBPF_HELPER_RET_TYPE;
typedef uint8_t UK_EBPF_HELPER_RET_TYPE_t;

typedef enum UK_EBPF_HELPER_ARG_TYPE {
    DUMMY = 0,
} __attribute__((__packed__)) UK_EBPF_HELPER_ARG_TYPE;
typedef uint8_t UK_EBPF_HELPER_ARG_TYPE_t;

typedef uint8_t UK_EBPF_HELPER_ARG_TYPE_NUM_t;

#define UK_EBPF_HELPER_FUNCTION_MAX_NAME_LENGTH 256

typedef struct HelperFunctionSignature {
    char *m_function_name;
    UK_EBPF_HELPER_RET_TYPE_t m_return_type;
    UK_EBPF_HELPER_ARG_TYPE_NUM_t m_num_args;
    UK_EBPF_HELPER_ARG_TYPE_t *m_arg_types;
} HelperFunctionSignature;

typedef struct HelperFunctionEntry {
    const void *m_function_addr;
    HelperFunctionSignature m_function_signature;
} HelperFunctionEntry;

typedef struct HelperFunctionList {
    size_t m_length;
    size_t m_capacity;

    void (*m_entry_init)(HelperFunctionEntry *self, const char *functionName, const void *functionAddr,
                         const UK_EBPF_HELPER_RET_TYPE_t retType,
                         const UK_EBPF_HELPER_ARG_TYPE_NUM_t argTypeCount, const UK_EBPF_HELPER_ARG_TYPE_t argTypes[]);

    void (*m_entry_destructor)(struct HelperFunctionEntry *self);

    HelperFunctionEntry *m_entries;
} HelperFunctionList;

struct HelperFunctionList *
helper_function_list_init(size_t capacity,
                          void (*entry_constructor)(HelperFunctionEntry *, const char *, const void *,
                                                    const UK_EBPF_HELPER_RET_TYPE_t,
                                                    const UK_EBPF_HELPER_ARG_TYPE_NUM_t, const UK_EBPF_HELPER_ARG_TYPE_t[]),
                          void (*destruct_entry)(struct HelperFunctionEntry *));

bool helper_function_list_resize(HelperFunctionList *self, size_t new_capacity);

bool helper_function_list_push_back(HelperFunctionList *self,
                                    const char *functionName, const void *functionAddr, UK_EBPF_HELPER_RET_TYPE_t retType,
                                    UK_EBPF_HELPER_ARG_TYPE_NUM_t argTypeCount, const UK_EBPF_HELPER_ARG_TYPE_t argTypes[]);

void helper_function_list_apply_function(HelperFunctionList *self, void (*apply)(struct HelperFunctionEntry *));

void helper_function_list_remove_elem(HelperFunctionList *self, const char *label);

void helper_function_list_print(HelperFunctionList *self, void (*printer)(struct HelperFunctionEntry *));

void helper_function_list_destroy(HelperFunctionList *self);

#endif /* HELPER_FUNCTION_LIST_H */
