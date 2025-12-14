# File Sharing System - Refactoring & Implementation Report

**Project:** FileSharingSystem  
**Date:** 2025-12-14  
**Status:** Upload Module Implemented (Basic)

---

## 📋 Table of Contents
1. [Current Implementation Status](#current-implementation-status)
2. [Stream Transmission Mechanism](#stream-transmission-mechanism)
3. [Refactoring Work Done](#refactoring-work-done)
4. [Known Limitations](#known-limitations)
5. [Future Improvements Needed](#future-improvements-needed)
6. [Build Instructions](#build-instructions)

---

## 🎯 Current Implementation Status

### ✅ Implemented (Working)
- **Client Upload Module** (`client/lib/file_ops/`)
  - Chunk-based file streaming (4KB chunks)
  - Basic handshake protocol with server
  - File size calculation and transmission
  - Error handling for file not found

- **Server Upload Handler** (`server/lib/file_ops/`)
  - Receive file chunks from client
  - Write to storage directory
  - Handshake confirmation (UPLOAD_READY)
  - Completion confirmation (UPLOAD_COMPLETE)
  - Directory traversal protection (`..` check)

- **Modular Architecture**
  - Clean separation: `lib/` for business logic, `src/` for orchestration
  - Reusable modules following structure.md guidelines

### ❌ Not Implemented (Removed/Pending)
- **Download functionality** - Completely removed from both client and server
- Advanced error handling (retry, resume)
- Progress bar/percentage display
- File integrity checks (checksums)
- Concurrent upload protection (race condition handling)
- Large file optimization (sendfile syscall)

---

## 🔄 Stream Transmission Mechanism

### Overview
Files are transmitted using **chunk-based streaming** over TCP sockets. This ensures large files can be transferred without loading them entirely into RAM.

### Upload Flow (Client → Server)

```
CLIENT                          SERVER
  │                               │
  │  1. UPLOAD <file> <size>      │
  ├──────────────────────────────>│
  │                               │ Check permissions
  │                               │ Create file (fopen)
  │                               │
  │     2. OK UPLOAD_READY        │
  │<──────────────────────────────┤
  │                               │
  │  3. [Chunk 1: 4KB binary]     │
  ├──────────────────────────────>│ fwrite(chunk → disk)
  │                               │
  │  4. [Chunk 2: 4KB binary]     │
  ├──────────────────────────────>│ fwrite(chunk → disk)
  │                               │
  │  ... (loop until filesize)    │
  │                               │
  │  N. [Last chunk: <4KB]        │
  ├──────────────────────────────>│ fwrite(last chunk)
  │                               │ fclose(file)
  │                               │
  │   OK UPLOAD_COMPLETE          │
  │<──────────────────────────────┤
  │                               │
```

### Technical Details

**Client Side (`file_upload.c`):**
```c
// Step 1: Read file in 4KB chunks
while ((bytes_read = fread(chunk, 1, CHUNK_SIZE, file)) > 0) {
    // Step 2: Send chunk immediately (don't buffer in RAM)
    send(client_socket, chunk, bytes_read, 0);
    total_sent += bytes_read;
}
```
- **Why 4KB?** Balance between:
  - Small enough: Reduces RAM usage
  - Large enough: Minimizes system calls (fread/send overhead)
  - Typical network MTU: ~1500 bytes, so 4KB fits well

**Server Side (`file_transfer.c`):**
```c
while (total_received < filesize) {
    // Calculate how much to read (prevent over-reading)
    long remaining = filesize - total_received;
    int to_read = (remaining < CHUNK_SIZE) ? remaining : CHUNK_SIZE;
    
    // Receive chunk from network
    bytes_received = recv(client_socket, buffer, to_read, 0);
    
    // Write immediately to disk (streaming, not buffering)
    fwrite(buffer, 1, bytes_received, file);
    total_received += bytes_received;
}
```
- **Critical Logic:** Must track `total_received` to know when to stop
- **Why not `recv` until 0?** Because TCP is a stream, not message-based. We need to know exact byte count.

### Protocol Used
- **Transport Layer:** TCP (`SOCK_STREAM`)
- **Application Layer:** Custom text-based protocol
- **Why TCP?**
  - ✅ Reliable: Automatic retransmission if packets lost
  - ✅ In-order delivery: Chunks arrive in correct sequence
  - ✅ Flow control: Prevents sender from overwhelming receiver
  - ❌ Slower than UDP (but file transfer needs reliability)

---

## 🔨 Refactoring Work Done

### Problem Before
```
client/src/client.c (240 lines)
├── Socket setup
├── Upload logic (inline, ~60 lines)
├── Download logic (inline, ~80 lines)
└── Main loop
```
- ❌ All logic in one file
- ❌ Cannot reuse upload code
- ❌ Hard to test
- ❌ Violates single responsibility principle

### Solution After
```
client/
├── lib/file_ops/
│   ├── include/file_upload.h    # Clean interface
│   └── src/file_upload.c         # Upload implementation
└── src/client.c (107 lines)     # Only orchestration
```

**Changes Made:**
1. Created `file_upload` module with single public function:
   ```c
   int file_upload(int client_socket, const char *filepath);
   ```
2. Removed all inline upload/download logic from `client.c`
3. Client now just calls: `file_upload(socket, path)`
4. Reduced from 240 → 107 lines (55% reduction)

**Server Side:**
- Already followed modular structure
- Removed `handle_download()` from `file_transfer.c`
- Removed `CMD_DOWNLOAD` case from `server.c`

---

## ⚠️ Known Limitations

### 1. Race Condition (CRITICAL)
**Problem:**
```
User A: UPLOAD file.txt → Server opens storage/file.txt (write mode)
User B: UPLOAD file.txt → Server opens storage/file.txt (OVERWRITES!)
Result: Corrupted file with mixed data from A and B
```

**Why it happens:**
- No file locking mechanism
- `fopen(path, "wb")` truncates existing file
- Multi-threaded server doesn't coordinate writes

**Impact:** Data corruption in concurrent scenarios

### 2. Incomplete Upload Handling
**Problem:**
- If network drops mid-transfer, server keeps partial file
- No cleanup of "junk" files
- Cannot resume upload from where it stopped

**Current Behavior:**
```c
if (total_received == filesize) {
    // Save file
} else {
    printf("Upload incomplete\n");
    // File STILL exists on disk (corrupted)
}
```

### 3. No Error Recovery
- Client `send()` failure → Silent exit (no retry)
- Server `recv()` timeout → Hangs forever (blocking I/O)
- No timeout mechanism

### 4. Security Issues
- Basic `..` check for directory traversal
- No authentication check (anyone can upload)
- No file size limit (can fill server disk)
- No virus scanning

### 5. Performance Not Optimal
- Uses `fread/send` (copies data multiple times):
  ```
  HDD → File Buffer → User Space → Kernel Space → Network Card
  ```
- Production systems use `sendfile()` (zero-copy):
  ```
  HDD → Network Card (direct)
  ```

---

## 🚀 Future Improvements Needed

### Priority 1 (Must Have)
- [ ] **Fix race condition:** 
  - Option A: UUID-based filenames (e.g., `user1_uuid123_file.txt`)
  - Option B: File locking with `flock()`
  - Option C: Separate directories per user
  
- [ ] **Cleanup on failure:**
  ```c
  if (total_received != filesize) {
      fclose(file);
      remove(full_path);  // Delete corrupted file
      return -1;
  }
  ```

- [ ] **Add timeouts:**
  ```c
  struct timeval timeout;
  timeout.tv_sec = 30;  // 30 second timeout
  setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  ```

### Priority 2 (Should Have)
- [ ] **Resume capability:**
  - Send offset in UPLOAD command
  - Server uses `fseek()` to append
  - Client sends from offset position

- [ ] **Progress tracking:**
  ```c
  printf("\rUploading: [%ld/%ld] %.2f%%", 
         total_sent, filesize, 
         (float)total_sent/filesize*100);
  ```

- [ ] **Checksum verification:**
  - Client sends MD5/SHA256 hash
  - Server calculates hash after receive
  - Compare to detect corruption

### Priority 3 (Nice to Have)
- [ ] Use `sendfile()` for zero-copy (Linux-specific)
- [ ] Compression (gzip) for text files
- [ ] Encryption (TLS/SSL)
- [ ] Rate limiting (prevent DoS)

---

## 🛠️ Build Instructions

### Client
```bash
mkdir -p client/bin
gcc -o client/bin/client \
    client/src/client.c \
    client/lib/file_ops/src/file_upload.c \
    -I client/include \
    -I client/lib/file_ops/include \
    -Wall -Wextra -O2

# Run
./client/bin/client
```

### Server
```bash
# (Refer to server Makefile)
make -C server
./server/bin/server
```

### Test Upload
```bash
# Terminal 1: Start server
./server/bin/server

# Terminal 2: Run client
./client/bin/client
> REGISTER user1 pass123
> LOGIN user1 pass123
> UPLOAD test.txt
```

---

## 📝 Summary for AI Context

**When reviewing this project later, AI should know:**

1. **What's working:** 
   - Basic upload with chunk streaming ✅
   - Modular architecture ✅
   - TCP-based reliable transfer ✅

2. **What's NOT working:**
   - Download (completely removed)
   - Concurrent upload safety (race condition exists)
   - Error recovery (no retry/resume)

3. **Key decisions made:**
   - Chunk size: 4KB (balance of RAM vs syscall overhead)
   - TCP protocol: Reliability over speed
   - Modular structure: Follows structure.md strictly

4. **Critical issues to address next:**
   - Race condition (file overwrite by concurrent users)
   - Incomplete file cleanup
   - Add download functionality (mirror upload design)

5. **Code locations:**
   - Client upload: `client/lib/file_ops/src/file_upload.c`
   - Server upload: `server/lib/file_ops/src/file_transfer.c`
   - Protocol handshake: `UPLOAD → UPLOAD_READY → [chunks] → UPLOAD_COMPLETE`
