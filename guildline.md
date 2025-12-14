# Role: Senior C Network Engineer & System Architect

**Context:**
I am building a **File Sharing System** using **C language** with a Client-Server architecture. The application runs on the Command Line Interface (CLI).
* **Goal:** Create a robust system where users can register, login, join groups, and share files within those groups via a custom network protocol.
* **Storage Strategy:**
    * **Metadata:** Users, Groups, and Membership data are stored in `.txt` files (flat-file database). You must handle concurrency (file locking) to prevent race conditions.
    * **Actual Files:** Stored in the server's local file system. Server paths map directly to group IDs.
* **Networking:**
    * Use **TCP Sockets** for reliability.
    * The architecture must handle multiple concurrent clients (suggest using `select()`, `poll()`, or `pthread`).
    * **Big File Support:** Critical requirement. Files must be transferred using chunk-based streaming (buffering), never loaded entirely into RAM.

---

## 1. Functional Requirements & Logic

### A. Authentication & Session
* **Users:** Register and Login using username/password.
* **Session:** Successful login returns a `session_id`.
* **Log:** The system must log activities.

### B. Group Management (Logic)
* **Roles:**
    * **Leader (Creator):** Full permissions (Delete files, Rename, Kick members).
    * **Member:** Can Upload, Download, Create sub-folders. Cannot delete/rename others' files.
* **Flow:** Users create groups or request to join. Leaders approve requests.

### C. File System Operations
* Operations: List Dir, Make Dir, Rename, Delete, Copy, Move.
* Pathing: Operations are based on the relative path inside the Group's folder on the server.

---

## 2. Protocol Definition (Custom Application Layer)
The communication follows a strict Request-Response model using string commands.
* **Success Response:** `OK <COMMAND> <DATA>`
* **Error Response:** `ERROR <ERR_MESSAGE>`

### Command Specifications:

**1. Authentication**
* `REGISTER <username> <password>`
* `LOGIN <username> <password>` -> Returns `<session_id>`

**2. Group Management**
* `CREATE_GROUP <group_name>` -> Returns `<group_id>`
* `LIST_GROUPS` -> Returns `ID:Name;ID:Name...`
* `LIST_MEMBERS <group_id>`
* `JOIN_REQ <group_id>`
* `APPROVE_JOIN <group_id> <user_id>` (Leader only)
* `INVITE_USER <group_id> <username>`
* `LEAVE_GROUP <group_id>`
* `KICK_MEMBER <session_id> <group_id> <user_id>` (Leader only)

**3. File/Folder Ops**
* `LIST_DIR <path>` -> Returns `File|Size|Date;Folder||Date...`
* `MKDIR <path>`
* `RENAME <old_path> <new_name>`
* `DELETE <path>`
* `COPY <src> <dest>`
* `MOVE <src> <dest>`

**4. File Transfer (CRITICAL)**
* **UPLOAD:**
    1.  Client: `UPLOAD <path> <filesize>`
    2.  Server: `OK UPLOAD_READY` (checks space/permissions)
    3.  Client: Sends binary data stream (chunks).
    4.  Server: `OK UPLOAD_COMPLETE` (after receiving exactly `filesize` bytes).
* **DOWNLOAD:**
    1.  Client: `DOWNLOAD <path>`
    2.  Server: `OK DOWNLOAD <filesize>`
    3.  Client: Prepares to receive.
    4.  Server: Sends binary data stream immediately.

---

## 3. Your Instructions
**DO NOT WRITE THE FULL CODE YET.**
This is a complex project. I want to build this module by module to ensure quality and error handling.

1.  **Analyze** the requirements above.
2.  **Confirm** you understand the protocol and the C networking constraints (especially handling large files and TCP fragmentation).
3.  **Wait** for my next prompt to start the implementation.

**Response format:**
If you understand, simply reply: *"I have analyzed the requirements. I am ready to act as your Senior C Network Engineer. Which module shall we implement first?"*