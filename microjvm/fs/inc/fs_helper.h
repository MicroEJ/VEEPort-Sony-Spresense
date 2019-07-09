/*
 * C
 *
 * Copyright 2014-2019 MicroEJ Corp. All rights reserved.
 * This library is provided in source code for use, modification and test, subject to license terms.
 * Any modification of the source code will break MicroEJ Corp. warranties on the whole library.
 *
 */

#ifndef FS_HELPER_H
#define FS_HELPER_H

/**
 * @file
 * @brief LLFS implementation over POSIX API.
 * @author @CCO_AUTHOR@
 * @version @CCO_VERSION@
 * @date @CCO_DATE@
 */

#ifdef __cplusplus
extern "C"
{
#endif

// TODO move in conf file
#define FS_WORKER_JOB_COUNT (4)
#define FS_WAITING_LIST_SIZE (16)
#define FS_WORKER_STACK_SIZE (256)
#define FS_WORKER_PRIORITY (100)
#define FS_PATH_LENGTH (64)
#define FS_IO_BUFFER_SIZE (128)

	// TODO: check where to declare this function and global
	int32_t LLFS_set_path_param(uint8_t *path, uint8_t *path_param);
	extern MICROEJ_ASYNC_WORKER_handle_t fs_worker;

	//TODO add comment on structure constraints

	typedef struct
	{
		uint8_t path[FS_PATH_LENGTH];
		int32_t result;
	} FS_path_operation_t;

	typedef struct
	{
		uint8_t path[FS_PATH_LENGTH];
		int64_t result;
	} FS_path64_operation_t;

	typedef struct
	{
		uint8_t path[FS_PATH_LENGTH];
		int32_t result;
		LLFS_date_t date;
	} FS_get_last_modified_t;

	typedef struct
	{
		uint8_t path[FS_PATH_LENGTH];
		int32_t result;
		int32_t error_code;
		char *error_message;
	} FS_create_t;

	typedef struct
	{
		uint8_t path[FS_PATH_LENGTH];
		int32_t result;
		uint8_t new_path[FS_PATH_LENGTH];
	} FS_rename_to_t;

	typedef struct
	{
		uint8_t path[FS_PATH_LENGTH];
		int64_t result;
		int32_t space_type;
	} FS_get_space_size;

	//TODO: merge with FS_get_last_modified_t
	typedef struct
	{
		uint8_t path[FS_PATH_LENGTH];
		int32_t result;
		LLFS_date_t date;
	} FS_set_last_modified_t;

	typedef struct
	{
		uint8_t path[FS_PATH_LENGTH];
		int32_t result;
		int32_t access;
	} FS_is_accessible_t;

	typedef struct
	{
		uint8_t path[FS_PATH_LENGTH];
		int32_t result;
		int32_t access;
		int32_t enable;
		int32_t owner;
	} FS_set_permission_t;

	typedef struct
	{
		uint8_t path[FS_PATH_LENGTH];
		int32_t result;
		uint8_t mode;
		int32_t error_code;
		char *error_message;
	} FS_open_t;

	typedef struct
	{
		int32_t directory_ID;
	} FS_directory_operation_t;

	typedef struct
	{
		int32_t directory_ID;
		int32_t result;
		uint8_t path[FS_PATH_LENGTH];
	} FS_read_directory_t;

	typedef struct
	{
		int32_t directory_ID;
		int32_t result;
	} FS_close_directory_t;

	typedef struct
	{
		int32_t file_id;
		uint8_t *data;
		int32_t length;
		int32_t result;
		int32_t error_code;
		char *error_message;
		uint8_t buffer[FS_IO_BUFFER_SIZE];
	} FS_write_read_t;

	typedef struct
	{
		int32_t file_id;
		int32_t result;
		int32_t error_code;
		char *error_message;
	} FS_close_t;

	typedef struct
	{
		int32_t file_id;
		int32_t n;
		int32_t skipped_count;
		int32_t result;
		int32_t error_code;
		char *error_message;
	} FS_skip_t;

	typedef struct
	{
		int32_t file_id;
		int32_t result;
		int32_t error_code;
		char *error_message;
	} FS_available_t;

	typedef union {
		FS_path_operation_t path_operation;
		FS_path64_operation_t path64_operation;
		FS_get_last_modified_t get_last_modified;
		FS_create_t create;
		FS_rename_to_t rename_to;
		FS_directory_operation_t directory_operation;
		FS_read_directory_t read_directory;
		FS_close_directory_t close_directory;
		FS_set_last_modified_t set_last_modified;
		FS_is_accessible_t is_accessible;
		FS_set_permission_t set_permission;
		FS_open_t open;
		FS_write_read_t write;
		FS_close_t close;
		FS_skip_t skip;
		FS_available_t available;
	} FS_worker_param_t;

void LLFS_IMPL_get_last_modified_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_set_read_only_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_create_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_open_directory_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_read_directory_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_close_directory_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_rename_to_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_get_length_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_exist_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_get_space_size_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_make_directory_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_is_hidden_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_is_directory_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_is_file_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_set_last_modified_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_delete_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_is_accessible_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_IMPL_set_permission_action(MICROEJ_ASYNC_WORKER_job_t *job);

void LLFS_File_IMPL_open_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_File_IMPL_write_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_File_IMPL_read_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_File_IMPL_close_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_File_IMPL_skip_action(MICROEJ_ASYNC_WORKER_job_t *job);
void LLFS_File_IMPL_available_action(MICROEJ_ASYNC_WORKER_job_t *job);

#ifdef __cplusplus
}
#endif

#endif /* FS_HELPER_H */
