#include "vfs.h"
#include "debug.h"

vfs_return_flag vfs_open(char path[VFS_PATH_MAX], vfs_flags_t flags, vfs_file_descriptor_t **out)
{
    return 0;
}

vfs_return_flag vfs_aux_parse_path(char path[VFS_PATH_MAX], vfs_path_t *out)
{
    int read_ptr = 0;
    int write_depth_ptr = 0;
    int write_ptr = 0;
    while (path[read_ptr] != '\0')
    {
        if (read_ptr > VFS_MAX_FILE_FOLDER_NAME)
            return VFS_ENAMETOOLONG;
        if (write_depth_ptr > VFS_MAX_DEPTH)
            return VFS_ENAMETOOLONG;

        // k_log("Processing %c\n", path[read_ptr]);
        switch (path[read_ptr])
        {
        case VFS_PATH_SEP_CHAR:
            // /./ path should be ignored
            if (write_ptr > 0)
            {
                out->path[write_depth_ptr][write_ptr] = '\0';
                write_depth_ptr++;
                write_ptr = 0;
            }
            break;
        case VFS_PATH_DOT:
            // if prev was also dot we decrement write_depth
            if (read_ptr > 0 && path[read_ptr - 1] == VFS_PATH_DOT)
            {
                write_depth_ptr--;
                if (write_depth_ptr < 0)
                    write_depth_ptr = 0;
                write_ptr = 0;
                break;
            }

            // when should we break
            // "/.", "/./", "/.."

            // when we should not break
            // ".soorkie, "/.soorkie", "/soorkie.", "soorkie.ext"

            // dot on left is handled
            uint8_t has_char_on_left = (read_ptr > 0) && path[read_ptr - 1] != VFS_PATH_SEP_CHAR;
            uint8_t has_char_on_right = path[read_ptr + 1] != VFS_PATH_SEP_CHAR && path[read_ptr + 1] != '\0';

            if (!has_char_on_left && !has_char_on_right)
            {
                break;
            }
            // we need to handle cases where ".files and files.ext"
        default:
            // k_log("write depth is %lu\n", write_depth_ptr);
            out->path[write_depth_ptr][write_ptr++] = path[read_ptr];
            break;
        }
        read_ptr++;
    }

    // close
    if (write_ptr > 0)
    {
        out->path[write_depth_ptr][write_ptr++] = '\0';
    }

    out->depth = (write_ptr > 0) ? write_depth_ptr + 1 : write_depth_ptr;
    return VFS_OK;
}