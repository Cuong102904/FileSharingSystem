# File Sharing System

A network-based file sharing application with client-server architecture, supporting TCP/UDP/TFTP protocols.

## Project Structure

```
FileSharingSystem/
├── server/          # Server application
│   ├── src/         # Source code (.c files)
│   ├── include/     # Header files (.h files)
│   └── test/        # Server tests
├── client/          # Client application (GTK GUI)
│   ├── src/         # Source code (.c files)
│   ├── include/     # Header files (.h files)
│   └── test/        # Client tests
└── database/        # PostgreSQL schema and scripts
```

## Quick Start

### Prerequisites
```bash
sudo apt-get install build-essential libpq-dev libgtk-3-dev pkg-config postgresql-server-dev-all
```

### Build
```bash
make all          # Build both server and client
make server       # Build server only
make client       # Build client only
```

### Database Setup
```bash
make database     # Initialize PostgreSQL database
```

### Run Tests
```bash
make test         # Run all tests
make test-server  # Run server tests only
make test-client  # Run client tests only
```

## Folder Guidelines

### `server/`
- **`src/`**: Implementation files (.c). Organize by module: `auth/`, `group/`, `file/`, `protocol/`, etc.
- **`include/`**: Header files (.h). Mirror `src/` structure for easy navigation.
- **`test/`**: Unit and integration tests. Run with `make test` from server directory.

### `client/`
- **`src/`**: Implementation files (.c). Separate GUI (`gui/`), network (`network/`), and protocol code.
- **`include/`**: Header files (.h). Match `src/` structure.
- **`test/`**: Client-side tests. Test network and protocol logic independently.

### `database/`
- **`schema.sql`**: Main database schema (3NF design).
- **`init.sql`**: Initial data and setup scripts.

## Development Workflow

1. **Server Development**: Work in `server/src/`, add headers to `server/include/`
2. **Client Development**: Work in `client/src/`, add headers to `client/include/`
3. **Database Changes**: Update `database/schema.sql`, then run `make database`
4. **Testing**: Add tests in respective `test/` folders, run with `make test`

## Key Features

- User authentication and session management
- Group creation and membership management
- File upload/download with large file support
- Hierarchical file system per group
- Role-based permissions (Admin/Member)
- Activity logging

## Build Output

- Server executable: `server/bin/server`
- Client executable: `client/bin/client`
- Object files: `server/obj/`, `client/obj/` (auto-generated)

## Clean Build

```bash
make clean        # Remove all build artifacts
```

