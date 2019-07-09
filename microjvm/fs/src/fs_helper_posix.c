/*
 * C
 *
 * Copyright 2019 Sony Corp . All rights reserved.
 * This Software has been designed by MicroEJ Corp and all rights have been transferred to Sony Corp.
 * Sony Corp. has granted MicroEJ the right to sub-licensed this Software under the enclosed license terms.
 */


/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <time.h>
#include <dirent.h>
#include "LLFS_impl.h"
#include "LLFS_File_impl.h"
#include "microej_async_worker.h"
#include "fs_helper.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define LLFS_NORMAL_PERMISSIONS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

    /**
 * Set the size of the file referenced by the given file descriptor into size_out.
 * Returns LLFS_NOK on error or LLFS_OK on success.
 */
    static int FS_size_of_file(int fd, uint64_t *size_out)
    {
        int res = LLFS_NOK;
        int cur = lseek(fd, 0, SEEK_CUR);
        if(cur == -1)
        {
            res = LLFS_NOK;
        }
        int size = lseek(fd,0, SEEK_END);
        if(size != -1)
        {
            *size_out = size;
            res = LLFS_OK;
        }
        if(lseek(fd, cur, SEEK_SET) == -1)
        {
            res == LLFS_NOK;
        }
        return res;
    }

    void LLFS_IMPL_get_last_modified_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_get_last_modified_t *params = (FS_get_last_modified_t *)job->params;
        uint8_t *path = (uint8_t *)&params->path;
        LLFS_date_t *out_date = &params->date;

        jint fs_err;
        struct stat buffer;
        struct tm *date = NULL;
        params->result = LLFS_NOK; // error by default

        fs_err = stat(path, &buffer);

        if (fs_err == 0)
        {
            date = localtime(&buffer.st_mtime);
            if (date != NULL)
            {
                out_date->second = date->tm_sec;
                out_date->minute = date->tm_min;
                out_date->hour = date->tm_hour;
                out_date->day = date->tm_mday;
                out_date->month = date->tm_mon;
                out_date->year = date->tm_year + 1900;
                params->result = LLFS_OK;
            }
        }
    }

    void LLFS_IMPL_set_read_only_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        // Not supported
        // CHMOD function not available
        FS_path_operation_t *params = (FS_path_operation_t *)job->params;
        params->result = LLFS_NOK;
    }

    void LLFS_IMPL_create_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_create_t *params = (FS_create_t *)job->params;
        uint8_t *path = (uint8_t *)&params->path;

        params->result = LLFS_NOK; // error by default

        /*
     * Open file with:
     * - force created: O_CREAT
     * - fail if directory: O_WRONLY
     * - fail if the file exists: O_EXCL
     */
        int fd = open(path, O_CREAT | O_EXCL | O_WRONLY);

        /* test return function */
        if (fd != -1)
        {
            if (close(fd) == 0)
            {
                params->result = LLFS_OK; // success
            }
            else
            {
                params->error_code = errno;
                params->error_message = strerror(errno);
            }
        }
        else
        {
            /* Error during file creation, check if this error occurs because the file already exists */
            if (errno == EEXIST)
            {
                params->result = LLFS_NOT_CREATED;
            }
            else
            {
                params->error_code = errno;
                params->error_message = strerror(errno);
            }
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] : create file %s (status %d)\n", __FILE__, __LINE__, path, params->result);
#endif
    }

    void LLFS_IMPL_open_directory_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_path_operation_t *params = (FS_path_operation_t *)job->params;
        uint8_t *path = (uint8_t *)&params->path;

        params->result = LLFS_NOK; // error by default

        DIR *dir = NULL;

        dir = opendir(path);
        if (dir != NULL)
        {
            params->result = (int32_t)dir;
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] open dir %s (status %d)\n", __FILE__, __LINE__, path, params->result);
#endif
    }

    void LLFS_IMPL_read_directory_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_read_directory_t *params = (FS_read_directory_t *)job->params;
        int32_t directory_ID = params->directory_ID;
        uint8_t *path = (uint8_t *)&params->path;

        params->result = LLFS_NOK; // error by default

        struct dirent *entry = readdir((DIR *)directory_ID);
        if (entry != NULL)
        {
            char *entry_name = (char *)entry->d_name;
            if (strlen(entry_name) < sizeof(params->path))
            {
                strcpy(path, entry_name);
                params->result = LLFS_OK;
            }
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] read dir result %s (status %d)\n", __FILE__, __LINE__, path, params->result);
#endif
    }

    void LLFS_IMPL_close_directory_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_close_directory_t *params = (FS_close_directory_t *)job->params;
        int32_t directory_ID = params->directory_ID;

        int fs_err = closedir((DIR *)directory_ID);

        if (fs_err == 0)
        {
            params->result = LLFS_OK;
        }
        else
        {
            params->result = LLFS_NOK;
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] close dir (status %d err %d errno %d)\n", __FILE__, __LINE__, params->result, fs_err, errno);
#endif
    }

    void LLFS_IMPL_rename_to_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_rename_to_t *params = (FS_rename_to_t *)job->params;
        uint8_t *path = (uint8_t *)&params->path;
        uint8_t *new_path = (uint8_t *)&params->new_path;

        int fs_err = rename(path, new_path);

        if (fs_err == 0)
        {
            params->result = LLFS_OK;
        }
        else
        {
            params->result = LLFS_NOK;
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] rename : old name %s, new name %s (status %d err %d errno %)\n", __FILE__, __LINE__, path, new_path, params->result, fs_err, errno);
#endif
    }

    void LLFS_IMPL_get_length_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_path64_operation_t *params = (FS_path64_operation_t *)job->params;
        uint8_t *path = (uint8_t *)&params->path;

        params->result = LLFS_NOK; // error by default

        struct stat buffer;
        int fs_err = stat(path, &buffer);
        if (fs_err == 0)
        {
            params->result = buffer.st_size;
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] length of %s : %ld (err %d errno %d)\n", __FILE__, __LINE__, path, params->result, fs_err, errno);
#endif
    }

    void LLFS_IMPL_exist_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_path_operation_t *params = (FS_path_operation_t *)job->params;
        uint8_t *path = (uint8_t *)&params->path;
        int32_t result = LLFS_NOK;

        struct stat buffer;
        int fs_err = stat(path, &buffer);

        if (fs_err == 0)
        {
            params->result = LLFS_OK;
        }
        else
        {
            params->result = LLFS_NOK;
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] exist file %s : %d (err %d errno %d)\n", __FILE__, __LINE__, path, params->result, fs_err, errno);
#endif
    }

    void LLFS_IMPL_get_space_size_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_get_space_size *params = (FS_get_space_size *)job->params;
        uint8_t *path = (uint8_t *)&params->path;
        int32_t space_type = params->space_type;

        params->result = LLFS_NOK; //error by default

        struct statfs buffer;
        if (statfs(path, &buffer) >= 0)
        {
            /* f_blocks, f_bfree and f_bavail are defined in terms of f_frsize */
            long scale_factor = (long)buffer.f_bsize;

            switch (space_type)
            {
            case LLFS_FREE_SPACE:
                params->result = ((unsigned long)buffer.f_bfree * scale_factor);
                break;
            case LLFS_TOTAL_SPACE:
                params->result = ((unsigned long)buffer.f_blocks * scale_factor);
                break;
            case LLFS_USABLE_SPACE:
                params->result = ((unsigned long)buffer.f_bavail * scale_factor);
                break;
            }
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] : get space %d size on %s : %ld \n", __FILE__, __LINE__, space_type, path, params->result);
#endif
    }

    void LLFS_IMPL_make_directory_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_path_operation_t *params = (FS_path_operation_t *)job->params;
        uint8_t *path = (uint8_t *)&params->path;

        int fs_err = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
        if (fs_err == 0)
        {
            params->result = LLFS_OK;
        }
        else
        {
            params->result = LLFS_NOK;
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] : mkdir %s (status %d err %d errno %d)\n", __FILE__, __LINE__, path, params->result, fs_err, errno);
#endif
    }

    void LLFS_IMPL_is_hidden_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_path_operation_t *params = (FS_path_operation_t *)job->params;
        uint8_t *path = (uint8_t *)&params->path;

        if (path[0] == '.')
        {
            params->result = LLFS_OK;
        }
        else
        {
            params->result = LLFS_NOK;
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] : is hidden %s (status %d)\n", __FILE__, __LINE__, path, params->result);
#endif
    }

    void LLFS_IMPL_is_directory_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_path_operation_t *params = (FS_path_operation_t *)job->params;
        uint8_t *path = (uint8_t *)&params->path;
        struct stat buffer;

        int fs_err = stat(path, &buffer);
        if (fs_err == 0 && S_ISDIR(buffer.st_mode))
        {
            params->result = LLFS_OK;
        }
        else
        {
            params->result = LLFS_NOK;
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] : is directory %s (status %d err %d errno %d)\n", __FILE__, __LINE__, path, params->result, fs_err, errno);
#endif
    }

    void LLFS_IMPL_is_file_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_path_operation_t *params = (FS_path_operation_t *)job->params;
        uint8_t *path = (uint8_t *)&params->path;
        struct stat buffer;

        int fs_err = stat(path, &buffer);
        if (fs_err == 0 && S_ISREG(buffer.st_mode))
        {
            params->result = LLFS_OK;
        }
        else
        {
            params->result = LLFS_NOK;
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] : is file %s (status %d err %d errno %d)\n", __FILE__, __LINE__, path, params->result, fs_err, errno);
#endif
    }

    void LLFS_IMPL_set_last_modified_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        // utime function not supported
        // Not applicable
        FS_set_last_modified_t *params = (FS_set_last_modified_t *)job->params;
        params->result = LLFS_NOK;
    }

    void LLFS_IMPL_delete_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_path_operation_t *params = (FS_path_operation_t *)job->params;
        uint8_t *path = (uint8_t *)&params->path;

        if (remove(path) == 0)
        {
            params->result = LLFS_OK;
        }
        else
        {
            params->result = LLFS_NOK;
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] : delete %s (status %d, errno %i)\n", __FILE__, __LINE__, path, params->result, errno);
#endif
    }

    void LLFS_IMPL_is_accessible_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_is_accessible_t *params = (FS_is_accessible_t *)job->params;
        uint8_t *path = (uint8_t *)&params->path;
        int32_t checked_access = params->access;

        params->result = LLFS_NOK; // error by default

        int mode; // mode for POSIX access

        switch (checked_access)
        {
        case LLFS_ACCESS_READ:
            mode = R_OK;
            break;

        case LLFS_ACCESS_WRITE:
            mode = W_OK;
            break;

        case LLFS_ACCESS_EXECUTE:
            mode = X_OK;
            break;

        default:
            // Unknown access
            return;
        }

        if (access(path, mode) == 0)
        {
            params->result = LLFS_OK;
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] : is accessible %s access %d (status %d)\n", __FILE__, __LINE__, path, checked_access, params->result);
#endif
    }

    void LLFS_IMPL_set_permission_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        //  CHMOD not implementend
        // Not applicable will always return an error
        FS_set_permission_t *params = (FS_set_permission_t *)job->params;
        params->result = LLFS_NOK;
    }

    void LLFS_File_IMPL_open_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_open_t *params = (FS_open_t *)job->params;
        uint8_t *path = (uint8_t *)&params->path;
        uint8_t mode = params->mode;

        mode_t open_mode;
        int fd;

        params->result = LLFS_NOK; // error by default
        params->error_code = LLFS_NOK;
        params->error_message = NULL;

        switch (mode)
        {
        case LLFS_FILE_MODE_READ:
            open_mode = O_RDONLY;
            break;

        case LLFS_FILE_MODE_WRITE:
            open_mode = O_CREAT | O_WRONLY | O_TRUNC;
            break;

        case LLFS_FILE_MODE_APPEND:
            open_mode = O_CREAT | O_WRONLY | O_APPEND;
            break;

        default:
            params->error_code = mode;
            params->error_message = "Invalid mode";
            return;
        }

        /* open file according to internal mode and internal permission*/
        fd = open(path, open_mode, LLFS_NORMAL_PERMISSIONS);
        /* test return function */
        if (fd == -1)
        {
            params->error_code = errno;
            params->error_message = strerror(errno);
        }
        else
        {
            params->result = fd; // no error
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] : open %s mode %d (status %d errno %d)\n", __FILE__, __LINE__, path, mode, params->result, params->error_code );
#endif
    }

    void LLFS_File_IMPL_write_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_write_read_t *params = (FS_write_read_t *)job->params;
        int32_t file_id = params->file_id;
        uint8_t *data = params->data;
        int32_t length = params->length;

        ssize_t written_count = write(file_id, data, length);

        // - written_count < 0 when an error is detected

        // - written_count == 0 when nothing was written (for instance if the FS is full);
        //     in this case, let's check that we indeed wanted to write more than 0 bytes
        //     to determine if something bad happened
        if (written_count < 0 || (written_count == 0 && length > 0))
        {
            params->result = LLFS_NOK; // error
            params->error_code = errno;
            params->error_message = strerror(errno);
        }
        else
        {
            params->result = written_count;
            // fsync needed
            int sync_ret = fsync(file_id);
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] write file content %d - %d bytes writen (status %d errno %d)\n", __FILE__, __LINE__, file_id, written_count, params->result, params->error_code);
#endif
    }

    void LLFS_File_IMPL_read_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_write_read_t *params = (FS_write_read_t *)job->params;
        int32_t file_id = params->file_id;
        uint8_t *data = params->data;
        int32_t length = params->length;

        ssize_t read_count = read(file_id, data, length);

        if (read_count < 0)
        {
            params->result = LLFS_NOK; // error
            params->error_code = errno;
            params->error_message = strerror(errno);
        }
        else if (read_count == 0)
        {
            params->result = LLFS_EOF; // EOF
        }
        else
        {
            params->result = read_count;
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] read file content %d - %d bytes to read (status %d errno %d)\n", __FILE__, __LINE__, file_id, length, params->result, params->error_code);
#endif
    }

    void LLFS_File_IMPL_close_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_close_t *params = (FS_close_t *)job->params;
        int32_t file_id = params->file_id;

        int fs_err = close(file_id);

        if (fs_err != 0)
        {
            params->result = LLFS_NOK;
            params->error_code = errno;
            params->error_message = strerror(errno);
        }
        else
        {
            params->result = LLFS_OK;
        }

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] close file %d (status %d errno %d)\n", __FILE__, __LINE__, file_id, params->result, params->error_code);
#endif
    }

    void LLFS_File_IMPL_skip_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_skip_t *params = (FS_skip_t *)job->params;
        int32_t file_id = params->file_id;
        int64_t n = params->n;

        // Basically do a lseek with the given offset.
        // Before, checks the value of the offset:
        //	- don't go out of the bounds of the file:
        //		- compare the new position (current+offset) with the size of the file
        // 	- can be cast on the type defined by lseek

        int fs_err;
        uint64_t file_size;

        // Get the size of the file
        fs_err = FS_size_of_file(file_id, &file_size);
        if (fs_err == LLFS_OK)
        {
            // No error when computing the size of the file.

            //Get current file position.
            off_t lseek_err = lseek(file_id, 0, SEEK_CUR);
            if (lseek_err != (off_t)-1)
            {
                long current_position = lseek_err;
                //Saturate the offset to limit the new position to the size of the file.
                uint64_t max_offset = file_size - current_position;
                if (n > max_offset)
                {
                    n = max_offset;
                }

                // Convert given offset in a type accepted by lseek
                off_t offset = (off_t)n;

                // Check if the conversion from long long int to off_t is correct
                if (offset < 0 || offset != n)
                {
                    // An overflow occurs, saturate the value
                    offset = INT32_MAX;
                }

                // Skip the bytes !
                lseek_err = lseek(file_id, offset, SEEK_CUR);
                if (lseek_err != ((off_t)-1))
                {
                    // Skip done
                    params->skipped_count = lseek_err - current_position;
                    params->n = offset;
                    params->result = LLFS_OK;

#ifdef LLFS_DEBUG
                    printf("LLFS_DEBUG [%s:%u] skip %ld bytes on %d (status %d skip count %ld)\n", __FILE__, __LINE__, offset, file_id, params->result, params->skipped_count);
#endif
                    return;
                }
            }
        }
        // Error occurred
        params->skipped_count = 0;
        params->result = LLFS_NOK;
        params->error_code = errno;
        params->error_message = strerror(errno);
#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] skipped %ld bytes on %d (status %d errno %d)\n", __FILE__, __LINE__, params->skipped_count, file_id, params->result, params->error_code );
#endif
    }

    void LLFS_File_IMPL_available_action(MICROEJ_ASYNC_WORKER_job_t *job)
    {
        FS_available_t *params = (FS_available_t *)job->params;
        int32_t file_id = params->file_id;

        params->result = LLFS_NOK; // error by default
        params->error_code = LLFS_NOK;
        params->error_message = NULL;

        int fs_err;
        uint64_t file_size;

        // Get file size
        fs_err = FS_size_of_file(file_id, &file_size);
        if (fs_err == 0)
        {
            // Get current position
            off_t lseek_err = lseek(file_id, 0, SEEK_CUR);
            if (lseek_err != (off_t)-1)
            {
                long current_position = lseek_err;
                uint32_t available = file_size - current_position;
                params->result = (int)available;
            }
            else
            {
                // error during ftell: just update fs_err for debug/verbose.
                fs_err = (int)lseek_err;
            }
        }
        // else: error when computing the size of the file.

#ifdef LLFS_DEBUG
        printf("LLFS_DEBUG [%s:%u] available %d bytes on %d (errno %d)\n", __FILE__, __LINE__, params->result, file_id, params->error_code);
#endif
    }

#ifdef __cplusplus
}
#endif
