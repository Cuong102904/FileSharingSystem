#ifndef DIRECTORY_OPS_H
#define DIRECTORY_OPS_H

/**
 * Create directory recursively
 * @param full_path Full path to create (e.g., "storage/123/docs/reports")
 * @return 0 on success, -1 on error
 */
int create_directory_recursive(const char *full_path);

/**
 * Copy file or directory
 * @param src_path Source path
 * @param dst_path Destination path
 * @return 0 on success, -1 on error
 */
int copy_path(const char *src_path, const char *dst_path);

/**
 * Move file or directory (rename or copy+delete)
 * @param src_path Source path
 * @param dst_path Destination path
 * @return 0 on success, -1 on error
 */
int move_path(const char *src_path, const char *dst_path);

/**
 * Check if destination parent directory exists
 * @param dst_path Destination file path
 * @return 0 if parent exists, -1 if not
 */
int check_parent_directory_exists(const char *dst_path);

/**
 * Check if path has file extension (contains '.')
 * @param path Path to check
 * @return 1 if has extension, 0 otherwise
 */
int is_file_path(const char *path);

/**
 * Check if path is a directory (no extension)
 * @param path Path to check
 * @return 1 if is directory path, 0 otherwise
 */
int is_folder_path(const char *path);

/**
 * Validate actual filesystem type matches expectation
 * @param full_path Full path to check
 * @param expect_file 1 to expect file, 0 to expect directory
 * @return 0 if matches, -1 if doesn't match or doesn't exist
 */
int validate_path_type(const char *full_path, int expect_file);

/**
 * Delete directory recursively
 * @param path Full path to directory
 * @return 0 on success, -1 on error
 */
int delete_directory_recursive(const char *path);

#endif // DIRECTORY_OPS_H
