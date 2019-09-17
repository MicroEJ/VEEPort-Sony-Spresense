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

#ifndef MICROEJ_LIST_H
#define MICROEJ_LIST_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdio.h>

#include "microej_list_configuration.h"

/** @brief Success */
#define MICROEJ_LIST_SUCCESS						(0)
/** @brief Unknown error (should not occur) */
#define MICROEJ_LIST_ERROR_UNKNOWN					(-1)
/** @brief NULL parameter error */
#define MICROEJ_LIST_ERROR_NULL_PARAMETER			(-2)
/** @brief Memory allocation error */
#define MICROEJ_LIST_ERROR_NOMEM					(-3)

/**@brief Assert macro */
#ifdef MICROEJ_LIST_ENABLE_ASSERT
#define MICROEJ_LIST_ASSERT(x) do { if (!(x)) { MICROEJ_LIST_ASSERT_TRACE("%s, %d\n", __FILE__, __LINE__); while(1); } } while (0)
#else
#define MICROEJ_LIST_ASSERT(x) do { if (!(x)) { MICROEJ_LIST_ASSERT_TRACE("%s, %d\n", __FILE__, __LINE__); } } while (0)
#endif

#ifndef MICROEJ_LIST_LOG_LEVEL
    #error "MICROEJ_LIST_LOG_LEVEL must be defined"
#endif

/**@brief Debug logger */
#if (MICROEJ_LIST_LOG_DEBUG >= MICROEJ_LIST_LOG_LEVEL)
    #define MICROEJ_LIST_DEBUG_TRACE        printf("[MICROEJ_LIST][DEBUG] ");printf
#else
    #define MICROEJ_LIST_DEBUG_TRACE(...)   ((void) 0)
#endif

/**@brief Info logger */
#if (MICROEJ_LIST_LOG_INFO >= MICROEJ_LIST_LOG_LEVEL)
    #define MICROEJ_LIST_INFO_TRACE         printf("[MICROEJ_LIST][INFO] ");printf
#else
    #define MICROEJ_LIST_INFO_TRACE(...)    ((void) 0)
#endif

/**@brief Warning logger */
#if (MICROEJ_LIST_LOG_WARNING >= MICROEJ_LIST_LOG_LEVEL)
    #define MICROEJ_LIST_WARNING_TRACE      printf("[MICROEJ_LIST][WARNING] ");printf
#else
    #define MICROEJ_LIST_WARNING_TRACE(...) ((void) 0)
#endif

/**@brief Error logger */
#if (MICROEJ_LIST_LOG_ERROR >= MICROEJ_LIST_LOG_LEVEL)
    #define MICROEJ_LIST_ERROR_TRACE        printf("[MICROEJ_LIST][ERROR] ");printf
#else
    #define MICROEJ_LIST_ERROR_TRACE(...)   ((void) 0)
#endif

/**@brief Assert logger */
#if (MICROEJ_LIST_LOG_ASSERT >= MICROEJ_LIST_LOG_LEVEL)
    #define MICROEJ_LIST_ASSERT_TRACE       printf("[MICROEJ_LIST][ASSERT] ");printf
#else
    #define MICROEJ_LIST_ASSERT_TRACE(...)  ((void) 0)
#endif

/** @brief Define a list node */
typedef struct microej_list_node {
	void *item;
	struct microej_list_node *next;
} microej_list_node_t;

/** @brief Define a list */
typedef struct {
	microej_list_node_t *first_node;
	microej_list_node_t *free_nodes;
	int32_t size;
} microej_list_t;

/*
 * @brief Declare a MicroEJ linked list with static size
 */
#define MICROEJ_LIST_declare(_name, _size) microej_list_node_t  _name ## free_nodes[_size];	\
		microej_list_t _name = {NULL, &_name ## free_nodes[0], _size};

/** @brief Define a function prototype used to compare a list node with a characteristic */
typedef int32_t (*microej_list_compare_functor_t)(void *node, void *characteristic);

/** @brief Define a function prototype used to know if a list node has to be deleted */
typedef int8_t (*microej_list_has_to_be_deleted_functor_t)(void *item);

/** @brief Define a function prototype used to visit a list */
typedef void (*microej_list_visit_item_functor_t)(void *item);

/** @brief Define a function prototype used to delete a node */
typedef void (*microej_list_free_item_functor_t)(void *ptr);

/** @brief Define a function prototype used to process a node */
typedef void (*microej_list_process_item_functor_t)(void *ptr);


/**
 * @brief Initialize a list.
 * @param[in] list List to initialize.
 * @return MICROEJ_LIST_SUCCESS on success,
 * 		   MICROEJ_LIST_ERROR_NOMEM on memory allocation failure,
 *         MICROEJ_LIST_ERROR_UNKNOWN on unknown error.
 */
int32_t microej_list_initialize(microej_list_t* list);

/**
 * @brief Add an item in a list.
 * @param[in] list List on which the item has to be added.
 * @param[in] item Item to add.
 * @return MICROEJ_LIST_SUCCESS on success,
 * 		   MICROEJ_LIST_ERROR_NOMEM on memory allocation failure,
 *         MICROEJ_LIST_ERROR_UNKNOWN on unknown error.
 */
int32_t microej_list_add(microej_list_t* list, void *item);

/**
 * @brief Delete an item in a list.
 * @param[in] list List on which the item has to be deleted.
 * @param[in] item Item to delete.
 * @return MICROEJ_LIST_SUCCESS on success,
 * 		   MICROEJ_LIST_ERROR_NOMEM on memory allocation failure,
 *         MICROEJ_LIST_ERROR_UNKNOWN on unknown error.
 */
int32_t microej_list_delete(microej_list_t* list, void *item);

/**
 * @brief Get an item in according to a comparison function and a characteristic.
 * @param[in] list List on which the item has to be added.
 * @param[in] compare_to functor on the comparison function.
 * @param[in] characteristic characteristic on which the comparison will occurred.
 * @param[out] item Pointer on the item to return. Cannot be NULL
 * @return Item list if found, NULL otherwise.
 */
void* microej_list_get(microej_list_t* list, microej_list_compare_functor_t compare_to, void* characteristic);

/**
 * @brief Launch a visit of a list.
 * @param[in] list List on which the visit will occurred.
 * @param[in] visit_func functor on the visitor function.
 * @return MICROEJ_LIST_SUCCESS on success,
 * 		   MICROEJ_LIST_ERROR_NOMEM on memory allocation failure,
 *         MICROEJ_LIST_ERROR_UNKNOWN on unknown error.
 */
int32_t microej_list_visit(microej_list_t* list, microej_list_visit_item_functor_t visit_func);

/**
 * @brief Launch a cleanup of a list.
 * @param[in] list List on which the visit will occurred.
 * @param[in] has_to_be_deleted_func function used to check if a node has to be deleted.
 * @param[in] free_item_func function used to free an item.
 * @return MICROEJ_LIST_SUCCESS on success,
 * 		   MICROEJ_LIST_ERROR_NOMEM on memory allocation failure,
 *         MICROEJ_LIST_ERROR_UNKNOWN on unknown error.
 */
int32_t microej_list_cleanup(microej_list_t* list, microej_list_has_to_be_deleted_functor_t has_to_be_deleted_func, microej_list_free_item_functor_t free_item_func);

/**
 * @brief Consume a list.
 * @param[in] list List on which the visit will occurred.
 * @param[in] function to apply to each element consumated
 * @param[in] free_item_func function used to free an item.
 * @return MICROEJ_LIST_SUCCESS on success,
 *         MICROEJ_LIST_ERROR_UNKNOWN on unknown error.
 */
int32_t microej_list_consume(microej_list_t* list, microej_list_process_item_functor_t process_func, microej_list_free_item_functor_t free_item_func);

#ifdef __cplusplus
}
#endif
#endif // MICROEJ_LIST_H
