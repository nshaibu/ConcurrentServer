#include "recyclable_fields_array.h"

struct recyclable_array recycle_arr;

void init_recyclable_array_object() {
    memset(recycle_arr.tid, '\0', MAX_CONN*sizeof(pthread_t));
    recycle_arr.list_length = 1;
    recycle_arr.indeces_of_used_fields = create_normal_queue();
    recycle_arr.indeces_of_reusable_fields = create_normal_queue();
}

void set_field_used(unsigned index) {
    normal_enqueue(recycle_arr.indeces_of_used_fields, index);
}

void set_field_reusable(unsigned index) {
    void *node = normal_remove_node(recycle_arr.indeces_of_used_fields , index);
    normal_destroy_node(node);
    normal_enqueue(recycle_arr.indeces_of_reusable_fields, index);
}

int get_usable_field_index() {
    int var = 0;
    void *node = NULL;
    
    if ( !normal_queue_isempty(recycle_arr.indeces_of_reusable_fields) ) {
        node = normal_dequeue(recycle_arr.indeces_of_reusable_fields);

        if ( node != NULL ) {
            var = node->data;
            normal_destroy_node(node);
            return var;
        }
    } else {
        recycle_arr.list_length += 1;
        return recycle_arr.list_length;
    }

    return -1;
}