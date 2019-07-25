/*
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

#ifndef MICROEJ_LIST_CONFIGURATION_H
#define MICROEJ_LIST_CONFIGURATION_H

/**@brief Log priority levels */
#define MICROEJ_LIST_LOG_DEBUG      0
#define MICROEJ_LIST_LOG_INFO       1
#define MICROEJ_LIST_LOG_WARNING    2
#define MICROEJ_LIST_LOG_ERROR      3
#define MICROEJ_LIST_LOG_ASSERT     4
#define MICROEJ_LIST_LOG_NONE       5

/** @brief Enable assert if defined */
#define MICROEJ_LIST_ENABLE_ASSERT

/**@brief Current log level */
#define MICROEJ_LIST_LOG_LEVEL MICROEJ_LIST_LOG_DEBUG

#endif // MICROEJ_LIST_CONFIGURATION_H
