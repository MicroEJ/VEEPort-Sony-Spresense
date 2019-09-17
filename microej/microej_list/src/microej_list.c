/*
 * C
 *
 * Copyright 2019 IS2T. All rights reserved.
 * This library is provided in source code for use, modification and test, subject to license terms.
 * Any modification of the source code will break IS2T warranties on the whole library.
 */

/**
 * @file
 * @brief MicroEJ linked list implementation.
 * @author @CCO_AUTHOR@
 * @version @CCO_VERSION@
 * @date @CCO_DATE@
 */

#include <stdint.h>

#include "microej_list.h"


static microej_list_node_t * microej_list_node_alloc(microej_list_t* list)
{
	MICROEJ_LIST_ASSERT(NULL != list);

	microej_list_node_t *tmp_node = NULL;
	if(NULL != list->free_nodes){
		tmp_node = list->free_nodes;
		list->free_nodes = tmp_node->next;
		return tmp_node;
	}
	// list size reached
	return NULL;
}

static void microej_list_node_free(microej_list_t* list, microej_list_node_t *node)
{
	MICROEJ_LIST_ASSERT(NULL != list);
	MICROEJ_LIST_ASSERT(NULL != node);

	node->next = list->free_nodes;
	list->free_nodes = node;
}


int32_t microej_list_initialize(microej_list_t* list)
{
	MICROEJ_LIST_ASSERT(NULL != list);
	MICROEJ_LIST_ASSERT(NULL != list->free_nodes);

	int32_t list_size = list->size;
	int32_t i = -1;
	for(i = -1; ++i < list_size-1;){
		list->free_nodes[i].next = &list->free_nodes[i + 1];
	}

	return MICROEJ_LIST_SUCCESS;
}

int32_t microej_list_add(microej_list_t* list, void *item)
{
	MICROEJ_LIST_ASSERT(NULL != list);
	MICROEJ_LIST_ASSERT(NULL != item);

	// allocate the node to add in the list
	microej_list_node_t *tmp_node = microej_list_node_alloc(list);
	if(NULL == tmp_node){
		MICROEJ_LIST_ERROR_TRACE("Memory allocation failure (size: %d at %s line %d)\n", sizeof(microej_list_node_t), __func__, __LINE__);
		return MICROEJ_LIST_ERROR_NOMEM;
	}

	// fill/initialize the node
	tmp_node->item = item;
	tmp_node->next = NULL;

	// add the node in the list
	if(NULL == list->first_node){
		// first node to add
		list->first_node = tmp_node;
	} else {
		// add the node at the end of the list
		microej_list_node_t *current_node_ptr = list->first_node;
		// jump to the end of the list
		while(NULL != current_node_ptr->next){
			current_node_ptr = current_node_ptr->next;
		}
		// add the node at the end of the list
		current_node_ptr->next = tmp_node;
	}

	return MICROEJ_LIST_SUCCESS;
}

int32_t microej_list_delete(microej_list_t* list, void *item)
{
	MICROEJ_LIST_ASSERT(NULL != list);
	MICROEJ_LIST_ASSERT(NULL != item);

	if(NULL != list->first_node){
		microej_list_node_t *current_node_ptr = list->first_node;
		if(item == current_node_ptr->item){
			// delete first node of the list
			list->first_node = list->first_node->next;
			microej_list_node_free(list, current_node_ptr);
			return MICROEJ_LIST_SUCCESS;
		} else {
			while(NULL != current_node_ptr->next){
				if(item == current_node_ptr->next->item){
					// node found, delete it
					microej_list_node_t *node_to_delete = current_node_ptr->next;
					current_node_ptr->next = node_to_delete->next;
					microej_list_node_free(list, node_to_delete);
					return MICROEJ_LIST_SUCCESS;
				}
				// node not found, jump to the next node
				current_node_ptr = current_node_ptr->next;
			}
		}
	}
	// node not find (empty list or not in the list)
	return MICROEJ_LIST_ERROR_UNKNOWN;
}

void* microej_list_get(microej_list_t* list, microej_list_compare_functor_t compare_to, void* characteristic)
{
	MICROEJ_LIST_ASSERT(NULL != list);
	MICROEJ_LIST_ASSERT(NULL != compare_to);
	// no need to check characteristic parameter here. NULL can be a right value

	if(NULL != list->first_node){
		microej_list_node_t *current_node_ptr = list->first_node;
		if(compare_to(current_node_ptr->item, characteristic)){
			// first list node is the right one
			return current_node_ptr->item;
		} else {
			// explore list entries to find the right one
			while(NULL != current_node_ptr->next){
				// jump to the next node
				current_node_ptr = current_node_ptr->next;
				// compare current item whith characteristic to determine if the item is the right one
				if(compare_to(current_node_ptr->item, characteristic)){
					// item found, return it
					return current_node_ptr->item;
				}
			}
		}
	}
	// node not find (empty list or not in the list)
	return NULL;
}

int32_t microej_list_visit(microej_list_t* list, microej_list_visit_item_functor_t visit_func)
{
	MICROEJ_LIST_ASSERT(NULL != list);
	MICROEJ_LIST_ASSERT(NULL != visit_func);

	microej_list_node_t *current_node_ptr = list->first_node;
	while(NULL != current_node_ptr){
		// visit the current list item by calling visitor function passed as argument on current item
		visit_func(current_node_ptr->item);
		current_node_ptr = current_node_ptr->next;
	}

	return MICROEJ_LIST_SUCCESS;
}

int32_t microej_list_cleanup(microej_list_t* list, microej_list_has_to_be_deleted_functor_t has_to_be_deleted_func, microej_list_free_item_functor_t free_item_func)
{
	MICROEJ_LIST_ASSERT(NULL != list);
	MICROEJ_LIST_ASSERT(NULL != has_to_be_deleted_func);
	MICROEJ_LIST_ASSERT(NULL != free_item_func);

	microej_list_node_t *current_node_ptr = list->first_node;
	// explore list and delete item if needed
	while(NULL != current_node_ptr){
		if(has_to_be_deleted_func(current_node_ptr->item)){
			// current node and associated item has to be deleted
			microej_list_node_t *tmp_node_ptr = current_node_ptr->next;
			void* tmp_node_item_ptr = current_node_ptr->item;
			microej_list_delete(list, tmp_node_item_ptr);
			free_item_func(tmp_node_item_ptr);
			// jump to the next node
			current_node_ptr = tmp_node_ptr;
		} else {
			// jump to the next node
			current_node_ptr = current_node_ptr->next;
		}
	}

	return MICROEJ_LIST_SUCCESS;
}


int32_t microej_list_consume(microej_list_t* list, microej_list_process_item_functor_t process_func, microej_list_free_item_functor_t free_item_func)
{
	MICROEJ_LIST_ASSERT(NULL != list);
	MICROEJ_LIST_ASSERT(NULL != process_func);
	MICROEJ_LIST_ASSERT(NULL != free_item_func);

	microej_list_node_t *current_node_ptr = list->first_node;
	// explore list and delete item if needed
	while(NULL != current_node_ptr){
			// current node and associated item has to be deleted
			microej_list_node_t *tmp_node_ptr = current_node_ptr->next;
			void* tmp_node_item_ptr = current_node_ptr->item;
            process_func(tmp_node_item_ptr );
			microej_list_delete(list, tmp_node_item_ptr);
			free_item_func(tmp_node_item_ptr);
			// jump to the next node
			current_node_ptr = tmp_node_ptr;
	}
	return MICROEJ_LIST_SUCCESS;
}
