#ifndef RECYCLABLE_FIELDS_ARRAY
#define RECYCLABLE_FIELDS_ARRAY

#include <pthread.h>

#include "../error_logs/errno_logs.h"
#include "../server.h"

struct recyclable_array {
    pthread_t tid[MAX_CONN];   /*The array to recycle the fields in it*/
    unsigned int list_length;     /*The number of used fields in the array*/
    normal_queue *indeces_of_used_fields;        /*A queue of all used fields*/
    normal_queue *indeces_of_reusable_fields;    /*A queue of all reusable fields*/    
};

extern struct recyclable_array recycle_arr;

void init_recyclable_array_object();     /*Initialize the global recyclable array object*/

void set_field_used(unsigned index);     /*set that a field is used*/

void set_field_reusable(unsigned index);  /*Set that a field is reusable*/

int get_usable_field_index();   /*get unused field or reusable field index to use*/

#endif
