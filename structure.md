# Cấu Trúc Project FileSharingSystem

## Tổng Quan

FileSharingSystem được tổ chức theo mô hình **modular architecture** với sự phân tách rõ ràng giữa:
- **Code đặc trưng cho server** (`src/`, `include/`)
- **Libraries tái sử dụng** (`lib/`)

## Cấu Trúc Thư Mục

```
FileSharingSystem/
├── server/
│   ├── include/              # Headers cho code đặc trưng server
│   │   └── server.h          # Main server declarations
│   │
│   ├── src/                  # Code đặc trưng cho server
│   │   └── server.c          # CHỈ main server loop, accept, routing
│   │
│   ├── lib/                  # Libraries chung, tái sử dụng được
│   │   ├── auth/             # Authentication library
│   │   │   ├── include/
│   │   │   │   └── auth.h
│   │   │   ├── src/
│   │   │   │   ├── register.c
│   │   │   │   ├── login.c
│   │   │   │   └── user.c
│   │   │   └── test/
│   │   │
│   │   ├── session/          # Session management library
│   │   │   ├── include/
│   │   │   │   └── session.h
│   │   │   ├── src/
│   │   │   │   └── session.c
│   │   │   └── test/
│   │   │
│   │   ├── protocol/         # Protocol parsing library
│   │   │   ├── include/
│   │   │   │   └── protocol.h
│   │   │   ├── src/
│   │   │   │   ├── parser.c
│   │   │   │   └── handlers.c
│   │   │   └── test/
│   │   │
│   │   ├── group/            # Group management library
│   │   │   ├── include/
│   │   │   │   └── group.h
│   │   │   ├── src/
│   │   │   │   ├── group.c
│   │   │   │   └── membership.c
│   │   │   └── test/
│   │   │
│   │   └── file_ops/         # File operations library
│   │       ├── include/
│   │       │   └── file_ops.h
│   │       ├── src/
│   │       │   ├── file_transfer.c
│   │       │   └── file_management.c
│   │       └── test/
│   │
│   └── test/
│
└── client/
    └── ...
```

---

## Phân Biệt Các Thư Mục

### 1. `server/lib/` - Libraries Tái Sử Dụng

**Mục đích:** Chứa các module có thể tái sử dụng, độc lập với business logic cụ thể của server.

**Đặc điểm:**
- Mỗi library là một module độc lập
- Có thể test riêng biệt
- Có thể sử dụng lại ở client hoặc project khác
- Mỗi lib có cấu trúc: `include/`, `src/`, `test/`

**Ví dụ các library:**
- `lib/auth/` - Xử lý authentication (register, login, user management)
- `lib/session/` - Quản lý sessions (tạo, validate, xóa session)
- `lib/protocol/` - Parse và format protocol messages
- `lib/group/` - Logic quản lý groups và memberships
- `lib/file_ops/` - File operations (upload, download, file management)

**Cách viết code trong `lib/`:**

```c
// lib/auth/include/auth.h
#ifndef AUTH_H
#define AUTH_H

// Chỉ khai báo interface, không có implementation
int auth_register(const char *username, const char *password);
int auth_login(const char *username, const char *password);
int auth_check_user_exists(const char *username);

#endif
```

```c
// lib/auth/src/register.c
#include "../include/auth.h"
#include <stdio.h>
#include <string.h>

// Implementation chi tiết
int auth_register(const char *username, const char *password) {
    // Logic đăng ký user
    // ...
    return 1; // Success
}
```

**Nguyên tắc:**
- Header file (`*.h`) chỉ chứa declarations
- Source files (`*.c`) chứa implementations
- Mỗi library có thể có nhiều source files nhưng chỉ 1 main header file
- Tên header file phải trùng với tên thư mục (ví dụ: `lib/auth/include/auth.h`)

---

### 2. `server/src/` - Code Đặc Trưng Cho Server

**Mục đích:** Chứa code gắn liền với server (network operations, orchestration).

**Đặc điểm:**
- Code này chỉ dùng cho server, không tái sử dụng được
- Gọi các functions từ `lib/`
- Xử lý network I/O, socket management, threading
- Điều phối các libraries

**Ví dụ: `server/src/server.c`**

```c
// server/src/server.c
#include "server.h"
#include <lib/auth/include/auth.h>
#include <lib/session/include/session.h>
#include <lib/protocol/include/protocol.h>

int main() {
    // 1. Setup socket, bind, listen
    int server_socket = setup_server();
    
    // 2. Accept connections và tạo threads
    while (1) {
        int client_socket = accept(server_socket, ...);
        pthread_create(..., handle_client, ...);
    }
}

void* handle_client(void *arg) {
    int client_socket = *(int*)arg;
    char buffer[BUFFER_SIZE];
    
    while (recv(client_socket, buffer, ...) > 0) {
        // Gọi protocol library để parse và handle
        protocol_handle_request(client_socket, buffer);
    }
}
```

**Nguyên tắc:**
- `server.c` chỉ chứa orchestration logic
- Không chứa business logic (để trong `lib/`)
- Gọi functions từ các libraries
- Xử lý network operations (socket, accept, send, recv)

---

### 3. `server/include/` - Headers Cho Code Đặc Trưng

**Mục đích:** Chứa header files cho code trong `src/`.

**Đặc điểm:**
- Chỉ dùng trong server
- Không phải library interface
- Declarations cho functions trong `src/`

**Ví dụ: `server/include/server.h`**

```c
// server/include/server.h
#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <pthread.h>

// Declarations cho server.c
int setup_server(void);
void* handle_client(void *arg);
int accept_connection(int server_socket);

#endif
```

**Nguyên tắc:**
- Chỉ chứa declarations cho code trong `src/`
- Không chứa implementation
- Include các system headers cần thiết

---

## Quy Tắc Viết Code

### 1. Khi Nào Đặt Code Vào `lib/`?

Đặt vào `lib/` khi:
- ✅ Function có thể tái sử dụng
- ✅ Logic độc lập với network operations
- ✅ Có thể test riêng biệt
- ✅ Có thể dùng ở client hoặc project khác

**Ví dụ:**
- `auth_register()` → `lib/auth/`
- `session_create()` → `lib/session/`
- `protocol_parse_command()` → `lib/protocol/`
- `group_create()` → `lib/group/`

### 2. Khi Nào Đặt Code Vào `src/`?

Đặt vào `src/` khi:
- ✅ Code gắn liền với server operations
- ✅ Xử lý socket, network I/O
- ✅ Orchestration logic (gọi các libraries)
- ✅ Threading, concurrency management

**Ví dụ:**
- `main()` → `src/server.c`
- `accept_connection()` → `src/server.c`
- `handle_client()` → `src/server.c`

### 3. Include Paths

**Trong `lib/`:**
```c
// lib/auth/src/register.c
#include "../include/auth.h"  // Relative path
```

**Trong `src/`:**
```c
// server/src/server.c
#include "server.h"                    // Header trong include/
#include <lib/auth/include/auth.h>     // Library header
#include <lib/session/include/session.h>
```

---

## Workflow Phát Triển

### 1. Tạo Library Mới

**Bước 1:** Tạo cấu trúc thư mục
```bash
mkdir -p server/lib/new_lib/{include,src,test}
```

**Bước 2:** Tạo header file
```c
// server/lib/new_lib/include/new_lib.h
#ifndef NEW_LIB_H
#define NEW_LIB_H

// Declarations
int new_lib_function(int param);

#endif
```

**Bước 3:** Implement trong source files
```c
// server/lib/new_lib/src/new_lib.c
#include "../include/new_lib.h"

int new_lib_function(int param) {
    // Implementation
    return 0;
}
```

**Bước 4:** Sử dụng trong `server.c`
```c
// server/src/server.c
#include <lib/new_lib/include/new_lib.h>

// Sử dụng function
new_lib_function(123);
```

### 2. Thêm Function Mới Vào Library Có Sẵn

**Ví dụ: Thêm function vào `lib/auth/`**

**Bước 1:** Thêm declaration vào header
```c
// lib/auth/include/auth.h
int auth_register(const char *username, const char *password);
int auth_login(const char *username, const char *password);
int auth_logout(const char *session_id);  // Function mới
```

**Bước 2:** Implement trong source file
```c
// lib/auth/src/logout.c (hoặc thêm vào file có sẵn)
#include "../include/auth.h"

int auth_logout(const char *session_id) {
    // Implementation
    return 1;
}
```

**Bước 3:** Sử dụng trong `server.c` hoặc handler
```c
#include <lib/auth/include/auth.h>

auth_logout(session_id);
```

---

## Best Practices

### 1. Tổ Chức Code

- ✅ Mỗi library có 1 main header file (tên trùng với thư mục)
- ✅ Tách source files theo chức năng (ví dụ: `register.c`, `login.c`)
- ✅ Đặt test files trong `test/` của mỗi library
- ✅ Không đặt business logic trong `server.c`

### 2. Naming Conventions

- **Library names:** lowercase, descriptive (ví dụ: `auth`, `session`, `protocol`)
- **Function names:** prefix với library name (ví dụ: `auth_register()`, `session_create()`)
- **Header guards:** `LIBNAME_H` (ví dụ: `AUTH_H`, `SESSION_H`)

### 3. Dependencies

- ✅ Libraries không nên phụ thuộc vào `src/`
- ✅ Libraries có thể phụ thuộc lẫn nhau (ví dụ: `protocol` có thể dùng `auth`)
- ✅ `src/` phụ thuộc vào `lib/`, không ngược lại

### 4. Error Handling

- ✅ Mỗi library nên có error codes riêng
- ✅ Return values rõ ràng (0 = success, -1 = error, etc.)
- ✅ Log errors ở level phù hợp

### 5. Testing

- ✅ Mỗi library có test riêng trong `test/`
- ✅ Test độc lập, không cần chạy server
- ✅ Test các edge cases và error conditions

---

## Ví Dụ Hoàn Chỉnh

### Tạo Authentication Library

**1. Tạo cấu trúc:**
```
server/lib/auth/
├── include/
│   └── auth.h
├── src/
│   ├── register.c
│   ├── login.c
│   └── user.c
└── test/
    └── test_auth.c
```

**2. Header file:**
```c
// lib/auth/include/auth.h
#ifndef AUTH_H
#define AUTH_H

#define AUTH_SUCCESS 0
#define AUTH_USER_EXISTS -1
#define AUTH_INVALID_CREDENTIALS -2
#define AUTH_DB_ERROR -3

int auth_register(const char *username, const char *password);
int auth_login(const char *username, const char *password, char *session_id);
int auth_check_user_exists(const char *username);

#endif
```

**3. Implementation:**
```c
// lib/auth/src/register.c
#include "../include/auth.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

extern pthread_mutex_t db_mutex;

int auth_register(const char *username, const char *password) {
    if (auth_check_user_exists(username)) {
        return AUTH_USER_EXISTS;
    }
    
    pthread_mutex_lock(&db_mutex);
    // Write to database
    FILE *file = fopen("database/users.txt", "a");
    fprintf(file, "%s %s\n", username, password);
    fclose(file);
    pthread_mutex_unlock(&db_mutex);
    
    return AUTH_SUCCESS;
}
```

**4. Sử dụng trong server:**
```c
// server/src/server.c
#include "server.h"
#include <lib/auth/include/auth.h>

void* handle_client(void *arg) {
    int client_socket = *(int*)arg;
    char buffer[BUFFER_SIZE];
    
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    
    // Parse command
    if (strncmp(buffer, "REGISTER", 8) == 0) {
        char username[50], password[50];
        sscanf(buffer, "REGISTER %s %s", username, password);
        
        int result = auth_register(username, password);
        if (result == AUTH_SUCCESS) {
            send(client_socket, "OK REGISTER", 11, 0);
        } else {
            send(client_socket, "ERROR Registration failed", 25, 0);
        }
    }
}
```

---

## Tóm Tắt

| Thư Mục | Mục Đích | Đặc Điểm |
|---------|----------|----------|
| `server/lib/` | Libraries tái sử dụng | Độc lập, có thể test riêng, dùng được ở nhiều nơi |
| `server/src/` | Code đặc trưng server | Network operations, orchestration |
| `server/include/` | Headers cho `src/` | Declarations cho server code |

**Nguyên tắc vàng:**
- Business logic → `lib/`
- Network/server code → `src/`
- Tách biệt rõ ràng, dễ maintain và test
