# File Sharing System

A modular TCP-based file sharing system with client-server architecture. Supports user authentication, session management, and chunk-based streaming file upload.

**Status:** 🚧 In Development - Upload module implemented, Download not yet available

---

## 📋 Current Features

✅ **Authentication**
  - User registration (`REGISTER`)
  - Login with session ID (`LOGIN`)
  - Logout (`LOGOUT`)

✅ **File Upload**
  - Chunk-based streaming (4KB chunks)
  - Large file support (no RAM limitation)
  - Handshake protocol (UPLOAD_READY → chunks → UPLOAD_COMPLETE)

❌ **Not Yet Implemented**
  - Download functionality
  - Group management
  - File permissions
  - Resume capability

---

## 🏗️ Project Structure

```
FileSharingSystem/
├── client/
│   ├── lib/file_ops/          # Upload module (reusable)
│   │   ├── include/file_upload.h
│   │   └── src/file_upload.c
│   └── src/client.c            # Main client (CLI)
│
├── server/
│   ├── lib/                    # Reusable libraries
│   │   ├── auth/              # Authentication (register, login)
│   │   ├── session/           # Session management
│   │   ├── protocol/          # Protocol parser
│   │   └── file_ops/          # File transfer (upload)
│   └── src/server.c            # Main server (accept, routing)
│
└── docs/
    ├── REFACTORING_SUMMARY.md  # Technical overview
    └── client_refactoring.md   # Detailed implementation docs
```

**Architecture:** Modular design following `structure.md`
- `lib/` = Reusable business logic
- `src/` = Network orchestration

---

## 🚀 Quick Start

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

### 1️⃣ Build the Project

```bash
make clean && make
```

This will build both server and client:
- Output: `bin/server`
- Output: `bin/client`

---

### 2️⃣ Run the Server

**Terminal 1: Start Server**
```bash
make run-server
```

Expected output:
```
Server modules initialized.
Server listening on port 8080...
```

Server will:
- Create `server/database/` folder for user data
- Create `server/storage/` folder for uploaded files
- Listen on `127.0.0.1:8080`

---

### 3️⃣ Run the Client

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
3. LOGOUT <session_id>
4. UPLOAD <local_filepath> [server_path]
5. QUIT (to exit)
==================================

Enter command:
```

---

## 📝 Usage Examples

### Example 1: Register and Login

```bash
# In client terminal
Enter command: REGISTER alice password123
Server response: OK REGISTER

Enter command: LOGIN alice password123
Server response: OK LOGIN a3f2b4c5...  # Session ID
Session ID saved: a3f2b4c5...
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

## 🔧 Advanced Usage

### Running on Different IP/Port

**Edit `client/src/client.c`:**
```c
// Line ~156
if (inet_pton(AF_INET, "192.168.1.100", &server_addr.sin_addr) <= 0) {
    //                   ^^^^^^^^^^^^^^ Change to your server IP
```

**Edit `server/include/server.h` or `server/src/server.c`:**
```c
#define PORT 8080  // Change to your desired port
```

Then recompile both client and server.

### Multiple Clients

You can run multiple clients simultaneously:

```bash
# Terminal 2
./client/bin/client
> LOGIN alice pass123
> UPLOAD file1.txt

# Terminal 3
./client/bin/client
> LOGIN bob pass456
> UPLOAD file2.txt
```

**⚠️ Warning:** Uploading same filename concurrently has a race condition bug (see Known Issues).

---

## 🐛 Known Issues

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

## 📚 Documentation

- **`REFACTORING_SUMMARY.md`** - Technical overview, stream transmission mechanism, limitations
- **`docs/client_refactoring.md`** - Detailed code walkthrough, implementation details
- **`structure.md`** - Project architecture guidelines
- **`guildline.md`** - Development requirements and protocol specification

---

## 🧪 Testing

### Test Basic Upload
```bash
# Terminal 1: Server
./server/bin/server

# Terminal 2: Client
./client/bin/client
> REGISTER test test123
> LOGIN test test123
> UPLOAD README.md

# Verify
ls -lh server/storage/README.md
```

### Test Large File
```bash
dd if=/dev/urandom of=random.bin bs=1M count=500  # Create 500MB file
./client/bin/client
> LOGIN test test123
> UPLOAD random.bin  # Should complete without errors
```

### Test Error Handling
```bash
> UPLOAD nonexistent.txt
# Expected: Error: File 'nonexistent.txt' not found.

> UPLOAD /etc/passwd
# Server should reject (security check)
```

---

## 🛠️ Development

### Adding a New Feature

**Example: Add file listing**

1. **Create library module:**
   ```bash
   mkdir -p server/lib/file_list/{include,src}
   ```

2. **Define interface in header:**
   ```c
   // server/lib/file_list/include/file_list.h
   void handle_list_files(int client_socket);
   ```

3. **Implement in source:**
   ```c
   // server/lib/file_list/src/file_list.c
   #include "../include/file_list.h"
   void handle_list_files(int client_socket) { ... }
   ```

4. **Add to protocol parser:**
   ```c
   // server/lib/protocol/src/parser.c
   case CMD_LIST: handle_list_files(socket); break;
   ```

5. **Update Makefile** to include new files

See `structure.md` for detailed modular architecture guidelines.

---

## 🧹 Cleanup

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

---

## 🎯 Roadmap

- [ ] Fix race condition (concurrent uploads)
- [ ] Add timeout handling (recv/send)
- [ ] Implement download functionality
- [ ] Add progress bar for uploads
- [ ] File integrity checks (MD5/SHA256)
- [ ] Resume capability for interrupted uploads
- [ ] Group-based file sharing
- [ ] File permissions and access control

---

## 📄 License

Educational project for Network Programming course.

---

## 🆘 Troubleshooting

### "Connection refused"
- **Cause:** Server not running
- **Solution:** Start server first: `./server/bin/server`

### "Address already in use"
- **Cause:** Port 8080 already occupied
- **Solution:** 
  ```bash
  # Find process using port 8080
  sudo lsof -i :8080
  # Kill it
  sudo kill -9 <PID>
  ```

### "File open error" on server
- **Cause:** Permission issues with `storage/` directory
- **Solution:**
  ```bash
  mkdir -p server/storage
  chmod 755 server/storage
  ```

### Compile errors
- **Cause:** Missing headers or wrong paths
- **Solution:** Ensure you're in correct directory and using exact build commands above

---

## 📞 Support

For technical details about implementation, see documentation in `docs/` folder.

For protocol specification, see `guildline.md`.

