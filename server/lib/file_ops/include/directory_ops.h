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

#endif // DIRECTORY_OPS_H
