#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "helper_function_list.h"

HelperFunctionEntry *entry_constructor(const UK_UBPF_INDEX_t index, const char *function_name,
                                       const void *function_addr,
                                       const ebpf_return_type_t ret_type,
                                       const UK_EBPF_HELPER_ARG_TYPE_NUM_t arg_type_count,
                                       const ebpf_argument_type_t arg_types[]) {
    HelperFunctionEntry *entry = malloc(sizeof(HelperFunctionEntry) + arg_type_count * sizeof(ebpf_argument_type_t));
    if (entry == NULL) {
        return NULL;
    }

    entry->m_index = index;
    entry->m_function_addr = function_addr;

    entry->m_function_signature.m_function_name = malloc(strlen(function_name) + 1);
    strncpy(entry->m_function_signature.m_function_name, function_name, strlen(function_name) + 1);
    entry->m_function_signature.m_return_type = ret_type;
    entry->m_function_signature.m_num_args = arg_type_count;
    for (int i = 0; i < arg_type_count; ++i) {
        entry->m_function_signature.m_arg_types[i] = arg_types[i];
    }

    return entry;
}

void entry_destructor(struct HelperFunctionEntry *entry) {
    free(entry->m_function_signature.m_function_name);
    free(entry);
}

void test_create_new_list() {
    HelperFunctionList *list = helper_function_list_init(entry_constructor, entry_destructor);
    assert(list);
    assert(list->m_length == 0);
    assert(list->m_head == NULL);
    assert(list->m_tail == NULL);
    assert(list->m_entry_constructor == entry_constructor);
    assert(list->m_entry_destructor == entry_destructor);
    helper_function_list_destroy(list);
}

void test_push_back_work() {
    HelperFunctionList *list = helper_function_list_init(entry_constructor, entry_destructor);
    ebpf_argument_type_t dummy_arg_types[] = {4, 5, 6};
    assert(helper_function_list_push_back(list, 1, "test", (void *) 42, 7,
                                          sizeof(dummy_arg_types) / sizeof(ebpf_argument_type_t), dummy_arg_types));

    assert(list->m_length == 1);
    assert(list->m_head != NULL && list->m_head == list->m_tail);
    assert(list->m_head->m_index == 1);
    assert(list->m_head->m_function_addr == (void *) 42);
    assert(strcmp(list->m_head->m_function_signature.m_function_name, "test") == 0);
    assert(list->m_head->m_function_signature.m_return_type == 7);
    assert(list->m_head->m_function_signature.m_num_args == 3);
    assert(list->m_head->m_function_signature.m_arg_types[0] == 4);
    assert(list->m_head->m_function_signature.m_arg_types[1] == 5);
    assert(list->m_head->m_function_signature.m_arg_types[2] == 6);

    helper_function_list_destroy(list);
}


HelperFunctionList *helper_generate_dummy_data() {
    HelperFunctionList *list = helper_function_list_init(entry_constructor, entry_destructor);
    ebpf_argument_type_t dummy_arg_types[] = {4, 5, 6};

    assert(helper_function_list_push_back(list, 11, "test0", (void *) 42, 0, sizeof(dummy_arg_types), dummy_arg_types));
    assert(helper_function_list_push_back(list, 12, "test1", (void *) 42, 1, sizeof(dummy_arg_types), dummy_arg_types));
    assert(helper_function_list_push_back(list, 13, "test2", (void *) 42, 2, sizeof(dummy_arg_types), dummy_arg_types));
    assert(helper_function_list_push_back(list, 14, "test3", (void *) 42, 3, sizeof(dummy_arg_types), dummy_arg_types));
    assert(helper_function_list_push_back(list, 15, "test4", (void *) 42, 4, sizeof(dummy_arg_types), dummy_arg_types));
    assert(helper_function_list_push_back(list, 16, "test5", (void *) 42, 5, sizeof(dummy_arg_types), dummy_arg_types));
    assert(helper_function_list_push_back(list, 17, "test6", (void *) 42, 6, sizeof(dummy_arg_types), dummy_arg_types));

    HelperFunctionEntry *elem = list->m_head;
    assert(strcmp(elem->m_function_signature.m_function_name, "test0") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test1") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test2") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test3") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test4") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test5") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test6") == 0);

    assert(list->m_head->m_index == 11);
    assert(list->m_tail->m_index == 17);

    int index = 0;
    for (HelperFunctionEntry *entry = list->m_head; entry != NULL; entry = entry->m_next) {
        assert(entry->m_index == 11 + index);
        index++;
    }

    assert(list->m_length == 7);
    assert(index == list->m_length);

    return list;
}

void test_independent_lists_work() {
    HelperFunctionList *list1 = helper_generate_dummy_data();
    HelperFunctionList *list2 = helper_generate_dummy_data();

    assert(list1->m_length == 7);
    assert(list2->m_length == 7);

    list1->m_head->m_function_signature.m_function_name[0] = 'a';
    assert(strcmp(list1->m_head->m_function_signature.m_function_name, "aest0") == 0);
    assert(strcmp(list2->m_head->m_function_signature.m_function_name, "test0") == 0);

    helper_function_list_destroy(list1);
    helper_function_list_destroy(list2);
}

void test_remove_from_front_work() {
    HelperFunctionList *list = helper_generate_dummy_data();
    helper_function_list_remove_elem(list, "test0");

    assert(list->m_length == 6);
    assert(strcmp(list->m_head->m_function_signature.m_function_name, "test1") == 0);
    assert(strcmp(list->m_tail->m_function_signature.m_function_name, "test6") == 0);

    helper_function_list_destroy(list);
}

void test_remove_from_back_work() {
    HelperFunctionList *list = helper_generate_dummy_data();
    helper_function_list_remove_elem(list, "test6");

    assert(list->m_length == 6);
    assert(strcmp(list->m_head->m_function_signature.m_function_name, "test0") == 0);
    assert(strcmp(list->m_tail->m_function_signature.m_function_name, "test5") == 0);

    helper_function_list_destroy(list);
}

void test_remove_from_middle_work_0() {
    HelperFunctionList *list = helper_generate_dummy_data();
    helper_function_list_remove_elem(list, "test2");
    helper_function_list_remove_elem(list, "test3");

    assert(list->m_length == 5);

    HelperFunctionEntry *elem = list->m_head;
    assert(strcmp(elem->m_function_signature.m_function_name, "test0") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test1") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test4") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test5") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test6") == 0);

    helper_function_list_destroy(list);
}


void test_remove_from_middle_work_1() {
    HelperFunctionList *list = helper_generate_dummy_data();
    helper_function_list_remove_elem(list, "test2");
    helper_function_list_remove_elem(list, "test4");

    assert(list->m_length == 5);

    HelperFunctionEntry *elem = list->m_head;
    assert(strcmp(elem->m_function_signature.m_function_name, "test0") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test1") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test3") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test5") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test6") == 0);

    helper_function_list_destroy(list);
}

void test_remove_from_empty_work() {
    HelperFunctionList *list = helper_generate_dummy_data();
    helper_function_list_remove_elem(list, "test100");
    helper_function_list_remove_elem(list, "test200");

    assert(list->m_length == 7);

    HelperFunctionEntry *elem = list->m_head;
    assert(strcmp(elem->m_function_signature.m_function_name, "test0") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test1") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test2") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test3") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test4") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test5") == 0);

    elem = elem->m_next;
    assert(strcmp(elem->m_function_signature.m_function_name, "test6") == 0);

    helper_function_list_destroy(list);
}

int main() {
    test_create_new_list();
    test_push_back_work();

    test_independent_lists_work();

    test_remove_from_front_work();
    test_remove_from_back_work();
    test_remove_from_middle_work_0();
    test_remove_from_middle_work_1();

    test_remove_from_empty_work();

    exit(EXIT_SUCCESS);
}
