#include <stdlib.h>
#include <assert.h>

#include "helper_function_list.h"

void entry_init_dummy(HelperFunctionEntry *self, const char *functionName, const void *functionAddr,
                      const UK_EBPF_HELPER_RET_TYPE_t retType,
                      const UK_EBPF_HELPER_ARG_TYPE_NUM_t argTypeCount, const UK_EBPF_HELPER_ARG_TYPE_t argTypes[]) {
    self->m_function_addr = functionAddr;
    self->m_function_signature.m_function_name = malloc(strlen(functionName) + 1);
    strncpy(self->m_function_signature.m_function_name, functionName, strlen(functionName)+1);

    self->m_function_signature.m_return_type = retType;
    self->m_function_signature.m_num_args = argTypeCount;
    self->m_function_signature.m_arg_types = malloc(argTypeCount * sizeof(UK_EBPF_HELPER_ARG_TYPE_t));
    for (int i = 0; i < argTypeCount; ++i) {
        self->m_function_signature.m_arg_types[i] = argTypes[i];
    }
}

void entry_destructor_dummy(struct HelperFunctionEntry *self) {
    free(self->m_function_signature.m_function_name);
    self->m_function_signature.m_function_name = NULL;

    free(self->m_function_signature.m_arg_types);
    self->m_function_signature.m_num_args = 0;
}

void test_create_new_list() {
    HelperFunctionList *list = helper_function_list_init(10, entry_init_dummy, entry_destructor_dummy);
    assert(list);
    assert(list->m_length == 0);
    assert(list->m_capacity == 10);
    assert(list->m_entries != NULL);
    assert(list->m_entry_init == entry_init_dummy);
    assert(list->m_entry_destructor == entry_destructor_dummy);
    helper_function_list_destroy(list);
}

void test_push_back_work() {
    HelperFunctionList *list = helper_function_list_init(10, entry_init_dummy, entry_destructor_dummy);
    UK_EBPF_HELPER_ARG_TYPE dummy_arg_types[] = {4, 5, 6};
    assert(helper_function_list_push_back(list, "test", (void *) 42, 7, sizeof(dummy_arg_types), dummy_arg_types));

    assert(list->m_length == 1);
    assert(list->m_capacity == 10);

    assert(list->m_entries[0].m_function_addr == (void *) 42);
    assert(list->m_entries[0].m_function_signature.m_return_type == 7);
    assert(list->m_entries[0].m_function_signature.m_num_args == 3);
    assert(list->m_entries[0].m_function_signature.m_arg_types[0] == 4);
    assert(list->m_entries[0].m_function_signature.m_arg_types[1] == 5);
    assert(list->m_entries[0].m_function_signature.m_arg_types[2] == 6);
    assert(strcmp(list->m_entries[0].m_function_signature.m_function_name, "test") == 0);
}

void test_resize_before_push_back_work() {
    HelperFunctionList *list = helper_function_list_init(0, entry_init_dummy, entry_destructor_dummy);
    assert(list->m_length == 0);
    assert(list->m_capacity == 0);
    assert(list->m_entries != NULL);

    UK_EBPF_HELPER_ARG_TYPE dummy_arg_types[] = {4, 5, 6};
    assert(helper_function_list_push_back(list, "test", (void *) 42, 7, sizeof(dummy_arg_types), dummy_arg_types));

    assert(list->m_length == 1);
    assert(list->m_capacity == 2);
}

void test_resize_work() {
    HelperFunctionList *list = helper_function_list_init(10, entry_init_dummy, entry_destructor_dummy);
    helper_function_list_resize(list, 5);
    assert(list->m_length == 0);
    assert(list->m_capacity == 5);
    assert(list->m_entries != NULL);

    helper_function_list_resize(list, 20);
    assert(list->m_length == 0);
    assert(list->m_capacity == 20);
    assert(list->m_entries != NULL);
}

void test_resize_0_work() {
    HelperFunctionList *list = helper_function_list_init(10, entry_init_dummy, entry_destructor_dummy);
    helper_function_list_resize(list, 0);
    assert(list->m_length == 0);
    assert(list->m_capacity == 0);
    assert(list->m_entries == NULL);
}

HelperFunctionList *helper_generate_dummy_data() {
    HelperFunctionList *list = helper_function_list_init(0, entry_init_dummy, entry_destructor_dummy);
    UK_EBPF_HELPER_ARG_TYPE dummy_arg_types[] = {4, 5, 6};

    assert(helper_function_list_push_back(list, "test0", (void *) 42, 0, sizeof(dummy_arg_types), dummy_arg_types));
    assert(helper_function_list_push_back(list, "test1", (void *) 42, 1, sizeof(dummy_arg_types), dummy_arg_types));
    assert(helper_function_list_push_back(list, "test2", (void *) 42, 2, sizeof(dummy_arg_types), dummy_arg_types));
    assert(helper_function_list_push_back(list, "test3", (void *) 42, 3, sizeof(dummy_arg_types), dummy_arg_types));
    assert(helper_function_list_push_back(list, "test4", (void *) 42, 4, sizeof(dummy_arg_types), dummy_arg_types));
    assert(helper_function_list_push_back(list, "test5", (void *) 42, 5, sizeof(dummy_arg_types), dummy_arg_types));
    assert(helper_function_list_push_back(list, "test6", (void *) 42, 6, sizeof(dummy_arg_types), dummy_arg_types));

    assert(strcmp(list->m_entries[0].m_function_signature.m_function_name, "test0") == 0);
    assert(strcmp(list->m_entries[1].m_function_signature.m_function_name, "test1") == 0);
    assert(strcmp(list->m_entries[2].m_function_signature.m_function_name, "test2") == 0);
    assert(strcmp(list->m_entries[3].m_function_signature.m_function_name, "test3") == 0);
    assert(strcmp(list->m_entries[4].m_function_signature.m_function_name, "test4") == 0);
    assert(strcmp(list->m_entries[5].m_function_signature.m_function_name, "test5") == 0);
    assert(strcmp(list->m_entries[6].m_function_signature.m_function_name, "test6") == 0);

    assert(list->m_length == 7);
    assert(list->m_capacity == 8);

    return list;
}

void test_independent_lists_work() {
    HelperFunctionList *list1 = helper_generate_dummy_data();
    HelperFunctionList *list2 = helper_generate_dummy_data();

    assert(list1->m_length == 7);
    assert(list1->m_capacity == 8);
    assert(list2->m_length == 7);
    assert(list2->m_capacity == 8);

    list1->m_entries[0].m_function_signature.m_function_name[0] = 'a';
    assert(strcmp(list1->m_entries[0].m_function_signature.m_function_name, "aest0") == 0);
    assert(strcmp(list2->m_entries[0].m_function_signature.m_function_name, "test0") == 0);

    list1->m_entries[0] = list1->m_entries[6];
    list1->m_length--;
    assert(strcmp(list1->m_entries[0].m_function_signature.m_function_name, "test6") == 0);


    helper_function_list_destroy(list1);
    helper_function_list_destroy(list2);
}

void test_remove_from_front_work() {
    HelperFunctionList *list = helper_generate_dummy_data();
    helper_function_list_remove_elem(list, "test0");

    assert(list->m_length == 6);
    assert(list->m_capacity == 8);
    assert(strcmp(list->m_entries[0].m_function_signature.m_function_name, "test1") == 0);
    assert(strcmp(list->m_entries[6].m_function_signature.m_function_name, "test6") == 0);
    helper_function_list_destroy(list);
}

void test_remove_from_back_work() {
    HelperFunctionList *list = helper_generate_dummy_data();
    helper_function_list_remove_elem(list, "test6");

    assert(list->m_length == 6);
    assert(list->m_capacity == 8);
    assert(strcmp(list->m_entries[0].m_function_signature.m_function_name, "test0") == 0);
    assert(strcmp(list->m_entries[5].m_function_signature.m_function_name, "test5") == 0);
}

void test_remove_from_middle_work_0() {
    HelperFunctionList *list = helper_generate_dummy_data();
    helper_function_list_remove_elem(list, "test2");
    helper_function_list_remove_elem(list, "test3");

    assert(list->m_length == 5);
    assert(list->m_capacity == 8);
    assert(strcmp(list->m_entries[0].m_function_signature.m_function_name, "test0") == 0);
    assert(strcmp(list->m_entries[1].m_function_signature.m_function_name, "test1") == 0);
    assert(strcmp(list->m_entries[2].m_function_signature.m_function_name, "test4") == 0);
    assert(strcmp(list->m_entries[3].m_function_signature.m_function_name, "test5") == 0);
    assert(strcmp(list->m_entries[4].m_function_signature.m_function_name, "test6") == 0);

}

void test_remove_from_middle_work_1() {
    HelperFunctionList *list = helper_generate_dummy_data();
    helper_function_list_remove_elem(list, "test2");
    helper_function_list_remove_elem(list, "test4");

    assert(list->m_length == 5);
    assert(list->m_capacity == 8);
    assert(strcmp(list->m_entries[0].m_function_signature.m_function_name, "test0") == 0);
    assert(strcmp(list->m_entries[1].m_function_signature.m_function_name, "test1") == 0);
    assert(strcmp(list->m_entries[2].m_function_signature.m_function_name, "test3") == 0);
    assert(strcmp(list->m_entries[3].m_function_signature.m_function_name, "test5") == 0);
    assert(strcmp(list->m_entries[4].m_function_signature.m_function_name, "test6") == 0);
}

void test_remove_from_empty_work() {
    HelperFunctionList *list = helper_generate_dummy_data();
    helper_function_list_remove_elem(list, "test100");
    helper_function_list_remove_elem(list, "test200");

    assert(list->m_length == 7);
    assert(list->m_capacity == 8);
    assert(strcmp(list->m_entries[0].m_function_signature.m_function_name, "test0") == 0);
    assert(strcmp(list->m_entries[1].m_function_signature.m_function_name, "test1") == 0);
    assert(strcmp(list->m_entries[2].m_function_signature.m_function_name, "test2") == 0);
    assert(strcmp(list->m_entries[3].m_function_signature.m_function_name, "test3") == 0);
    assert(strcmp(list->m_entries[4].m_function_signature.m_function_name, "test4") == 0);
    assert(strcmp(list->m_entries[5].m_function_signature.m_function_name, "test5") == 0);
    assert(strcmp(list->m_entries[6].m_function_signature.m_function_name, "test6") == 0);
}

int main() {
    test_create_new_list();
    test_push_back_work();
    test_resize_before_push_back_work();
    test_resize_work();
    test_resize_0_work();

    test_independent_lists_work();

    test_remove_from_front_work();
    test_remove_from_back_work();
    test_remove_from_middle_work_0();
    test_remove_from_middle_work_1();

    test_remove_from_empty_work();

    return 0;
}
