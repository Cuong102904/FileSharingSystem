# File Sharing System

A modular TCP-based file sharing system with client-server architecture. Supports user authentication, session management, and chunk-based streaming file upload.





## Project Structure

```
FileSharingSystem/
├── Makefile                    # Unified build system
├── client/
│   ├── include/               # Client headers
│   ├── lib/file_ops/          # File upload/download modules
│   │   ├── include/
│   │   └── src/
│   └── src/client.c           # Main client application
│
├── server/
│   ├── include/               # Server headers
│   ├── lib/                   # Modular server libraries
│   │   ├── auth/             # User authentication (register, login)
│   │   ├── session/          # Session management
│   │   ├── client_session/   # Client state tracking
│   │   ├── protocol/         # Command parser & handlers
│   │   ├── file_ops/         # File transfer operations
│   │   ├── group/            # Group management
│   │   ├── thread_pool/      # Worker thread pool
│   │   ├── logger/           # Logging system
│   │   └── utils/            # Utility functions
│   ├── src/server.c          # Main server (epoll event loop)
│   ├── database/             # User data storage (created at runtime)
│   └── storage/              # Uploaded files (created at runtime)
```

**Architecture:** Modular design with separation of concerns
- `lib/` = Reusable business logic modules
- `src/` = Network orchestration and event loop

---

## Dependencies

**Required Libraries:**
- `pthread` - POSIX threads for thread pool
- `sys/socket.h`, `arpa/inet.h` - TCP socket APIs
- `sys/epoll.h` (Linux) or `sys/event.h` (macOS) - I/O multiplexing
- Standard C library (stdio, stdlib, string, etc.)

**Build Tools:**
- GCC 7.0+ or compatible C compiler
- GNU Make

**Note:** No external dependencies required. Pure C implementation.

---

## Server Architecture

### TCP Server
- Non-blocking TCP socket listening on **port 8080**
- Supports up to **100 concurrent clients**
- Uses `SO_REUSEADDR` for quick restart

### epoll-based I/O Multiplexing
- **Event-driven architecture** using `epoll` (Linux) or `kqueue` (macOS)
- **EPOLLONESHOT mode** prevents race conditions
- Main thread handles `accept()` and `recv()`, delegates processing to thread pool
- Sockets re-armed after each command processing

### Thread Pool Design
- **10 worker threads** created at startup
- **Circular task queue** (max 1024 tasks)
- Mutex-protected with condition variables
- Workers block until tasks available
- Each task processes one client command

### Message Protocol
**Text-based command protocol:**
```
Format: COMMAND arg1 arg2 arg3...
Example: UPLOAD group1 /path/to/file.txt storage/
```

**Supported Commands (18+):**
- **Auth**: `REGISTER`, `LOGIN`, `LOGOUT`
- **Files**: `UPLOAD`, `DOWNLOAD`, `MKDIR`, `LS`, `COPYFILE`, `MOVEFILE`, `DELETEFILE`, `RENAMEFILE`
- **Folders**: `COPYFOLDER`, `MOVEFOLDER`, `DELETEFOLDER`, `RENAMEFOLDER`
- **Groups**: `CREATE_GROUP`, `LIST_GROUPS`, `LIST_MEMBERS`, `JOIN_REQ`, `APPROVE_JOIN`, `INVITE_USER`, `RESPOND_INVITE`, `LEAVE_GROUP`, `KICK_MEMBER`

**Response Format:**
```
Success: OK COMMAND_NAME [data]
Error: ERROR Description
```

### Command Handling Flow
1. `epoll_wait()` detects client data ready
2. Main thread `recv()` command into buffer
3. Task created with `{client_socket, buffer, epoll_fd}`
4. Task queued to thread pool
5. Worker thread:
   - Parses command using `sscanf()`
   - Validates login state
   - Dispatches to handler function
   - Sends response to client
   - Re-arms socket with `EPOLLONESHOT`

---

## Quick Start

### Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential gcc make
```

**Other Linux:**
```bash
# Ensure you have GCC and Make installed
gcc --version  # Should be 7.0+
make --version
```

---

### Build the Project

```bash
make clean && make
```

This will build both server and client:
- Output: `bin/server`
- Output: `bin/client`

---

### Run the Server

**Terminal 1: Start Server**
```bash
make run-server
```

Expected output:
```
Creating runtime directories...
✓ Setup complete (database/ and storage/)
Starting server...
Client session module initialized.
Server modules initialized.
Thread pool created with 10 workers
Server listening on port 8080 (IO Multiplexing + Thread Pool)
Event loop started (epoll)
```

When a client connects, you'll see:
```
Client connected: socket 6
```

Server will:
- Create `server/database/` folder for user data
- Create `server/storage/` folder for uploaded files
- Listen on `127.0.0.1:8080`

---

### Run the Client

**Terminal 2: Start Client**
```bash
make run-client
```

Expected output:
```
Connected to server successfully!
==================================
Available commands:
1. REGISTER <username> <password>
2. LOGIN <username> <password>
3. LOGOUT
4. CREATE_GROUP <group_name>
5. LIST_GROUPS
6. LIST_MEMBERS <group_name>
7. KICK_MEMBER <group_name> <user_name>
8. RESPOND_INVITE <group_name> <status>
9. JOIN_REQ <group_name>
10. APPROVE_JOIN <group_name> <user_name>
11. INVITE_USER <group_name> <user_name>
12. UPLOAD <group_name> <local_path> <remote_path>
13. LEAVE_GROUP <group_name>
14. DOWNLOAD <group_name> <path_on_server> <local_save_path>
15. MKDIR <group_name> <path>
16. LS <group_name> <path>
17. COPYFILE <group_name> <source_file> <dest_file>
18. COPYFOLDER <group_name> <source_folder> <dest_folder>
19. MOVEFILE <group_name> <source_file> <dest_folder>/
20. MOVEFOLDER <group_name> <source_folder> <dest_parent_folder>/
21. DELETEFILE <group_name> <file_path>
22. RENAMEFILE <group_name> <old_name> <new_name>
23. DELETEFOLDER <group_name> <folder_path>
24. RENAMEFOLDER <group_name> <old_name> <new_name>
*. QUIT (to exit)
==================================
```

---

## Usage Examples

### Example 1: Register and Login

```bash
# In client terminal
Enter command: REGISTER alice password123
Server response: OK REGISTER

Enter command: LOGIN alice password123
Server response: OK LOGIN alice
Logged in as: alice
```

### Example 2: Upload a File

**Prepare test file:**
```bash
echo "Hello from FileSharingSystem!" > test.txt
```

**Upload to default storage/:**
```bash
Enter command: UPLOAD test.txt storage/
Uploading 30 bytes...
Upload complete: 30 bytes sent
Server response: OK UPLOAD_COMPLETE
```

**Verify on server side:**
```bash
ls -lh server/storage/
cat server/storage/test.txt
# Output: Hello from FileSharingSystem!
```

### Example 3: Upload to Subdirectory

**Upload to organized folders:**
```bash
Enter command: UPLOAD document.pdf storage/group1/
Uploading 52428 bytes...
Upload complete: 52428 bytes sent
Server response: OK UPLOAD_COMPLETE
```

Server automatically creates `server/storage/group1/` directory!

**Verify:**
```bash
ls -lh server/storage/group1/document.pdf
```

### Example 4: Upload Large File

```bash
# Create 100MB test file
dd if=/dev/zero of=large.bin bs=1M count=100

# Upload (will stream in 4KB chunks)
Enter command: UPLOAD large.bin storage/
Uploading 104857600 bytes...
Upload complete: 104857600 bytes sent
Server response: OK UPLOAD_COMPLETE
```

---

## Advanced Usage

### Running on Different IP/Port

**Edit `client/src/client.c`:**
```c
// Line 63
if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
    //                   ^^^^^^^^^^ Change to your server IP (e.g., "192.168.1.100")
```

**Edit `server/include/server.h` or `server/src/server.c`:**
```c
#define PORT 8080  // Change to your desired port
```

Then recompile both client and server.

### Multiple Clients

You can run multiple clients simultaneously from different terminals or different machines:

```bash
# Terminal 2 (or Machine 1)
make run-client
> REGISTER alice pass123
> LOGIN alice pass123
> UPLOAD group1 file1.txt docs/

# Terminal 3 (or Machine 2)
make run-client
> REGISTER bob pass456
> LOGIN bob pass456
> UPLOAD group1 file2.txt docs/
```

**Warning:** Uploading same filename concurrently has a race condition bug (see Known Issues).

---

## Known Issues

### 1. Concurrent Upload Race Condition
**Problem:** Two users uploading same filename → File corruption
**Status:** Known bug, not fixed yet
**Workaround:** Don't upload same filename simultaneously

### 2. Incomplete Upload Cleanup
**Problem:** If upload fails, partial file remains in `storage/`
**Status:** Known limitation
**Workaround:** Manually delete corrupted files from `storage/`

### 3. No Download Function
**Status:** Removed from codebase, not yet reimplemented
**Plan:** Will add in future (mirror upload design)

See `REFACTORING_SUMMARY.md` for detailed technical issues.

---







## Cleanup

### Remove Build Artifacts
```bash
# Clean server
cd server && make clean

# Clean client
rm -f client/bin/client
```

### Reset Database and Storage
```bash
rm -rf server/database/*  # Remove all users
rm -rf server/storage/*   # Remove all uploaded files
```



