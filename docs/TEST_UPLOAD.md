# Test Upload Functionality - Quick Guide

## Chuẩn bị

### 1. Build Project
```bash
# Build Server
cd server
make clean && make
cd ..

# Build Client
cd client
make clean && make
cd ..
```

### 2. Tạo Test Files
```bash
# File nhỏ (text)
echo "Hello from FileSharingSystem!" > test_small.txt

# File 1MB
dd if=/dev/urandom of=test_1mb.bin bs=1M count=1

# File 10MB
dd if=/dev/urandom of=test_10mb.bin bs=1M count=10
```

---

## Test Scenarios

### Scenario 1: Basic Upload (Happy Path)

**Terminal 1: Start Server**
```bash
cd server
./bin/server

# Expected output:
# Server modules initialized.
# Server listening on port 8080...
```

**Terminal 2: Run Client**
```bash
cd client
./bin/client

# You will see welcome screen
```

**Test Commands:**
```bash
# 1. Register
REGISTER alice password123
# Expected: Server response: OK REGISTER

# 2. Login
LOGIN alice password123
# Expected: Server response: OK LOGIN <session_id>

# 3. Upload small file
UPLOAD test_small.txt
# Expected:
# Uploading 30 bytes...
# Upload complete: 30 bytes sent
# Server response: OK UPLOAD_COMPLETE
```

**Verify:**
```bash
# Terminal 3
cd server
ls -lh storage/
cat storage/test_small.txt
# Should see: Hello from FileSharingSystem!
```

---

### Scenario 2: Upload Binary File

```bash
# In client
UPLOAD test_1mb.bin

# Expected: Upload complete: 1048576 bytes sent
```

**Verify Integrity:**
```bash
# Check file size
ls -lh server/storage/test_1mb.bin
# Should be exactly 1.0M

# Check checksum (must match)
md5sum test_1mb.bin
md5sum server/storage/test_1mb.bin
# Both should be identical!
```

---

### Scenario 3: Upload Large File

```bash
# In client
UPLOAD test_10mb.bin

# Watch server terminal - you'll see:
# File received successfully: storage/test_10mb.bin (10485760 bytes)
```

---

### Scenario 4: Error Handling

**Test 1: File Not Found**
```bash
UPLOAD nonexistent.txt
# Expected: Error: File 'nonexistent.txt' not found.
```

**Test 2: Multiple Users Same File (Race Condition Bug)**
```bash
# Terminal 2 (Client 1)
./client
> LOGIN alice pass123
> UPLOAD test_large.txt

# Terminal 3 (Client 2) - IMMEDIATELY after
./client
> LOGIN bob pass456
> UPLOAD test_large.txt

# Result: File will be corrupted (known bug!)
# Verify:
md5sum test_large.txt
md5sum server/storage/test_large.txt
# Will be DIFFERENT (corruption proof)
```

---

## Automated Verification Script

Create `verify_upload.sh`:

```bash
#!/bin/bash

echo "=== Upload Verification ==="

# 1. Check if server is running
if ! nc -z 127.0.0.1 8080 2>/dev/null; then
    echo "❌ Server not running"
    exit 1
fi
echo "✓ Server is running"

# 2. Check uploaded files
if [ -f "server/storage/test_small.txt" ]; then
    echo "✓ test_small.txt uploaded"
    
    # Verify content
    if grep -q "Hello from FileSharingSystem!" server/storage/test_small.txt; then
        echo "  ✓ Content correct"
    else
        echo "  ❌ Content corrupted"
    fi
else
    echo "❌ test_small.txt not found"
fi

# 3. Check binary file integrity
if [ -f "server/storage/test_1mb.bin" ]; then
    echo "✓ test_1mb.bin uploaded"
    
    ORIG=$(md5sum test_1mb.bin | awk '{print $1}')
    UPLOADED=$(md5sum server/storage/test_1mb.bin | awk '{print $1}')
    
    if [ "$ORIG" == "$UPLOADED" ]; then
        echo "  ✓ Checksum matches"
    else
        echo "  ❌ Checksum mismatch (corrupted)"
    fi
else
    echo "❌ test_1mb.bin not found"
fi

echo ""
echo "Storage contents:"
ls -lh server/storage/
```

---

## Performance Test

### Test Upload Speed

```bash
# Create 100MB file
dd if=/dev/zero of=test_100mb.bin bs=1M count=100

# Upload and measure time
time ./client << EOF
LOGIN alice pass123
UPLOAD test_100mb.bin
QUIT
EOF

# Typical results on localhost:
# real    0m2.5s  => ~40 MB/s
# user    0m0.1s
# sys     0m0.4s
```

---

## Cleanup

```bash
# Remove test files
rm -f test_*.txt test_*.bin

# Clear server storage
rm -rf server/storage/*
rm -rf server/database/*
```

---

## Common Issues

### "Connection refused"
- **Cause:** Server not running
- **Fix:** Start server: `cd server && ./bin/server`

### "File open error" on server
- **Cause:** storage/ directory doesn't exist
- **Fix:** `mkdir -p server/storage`

### Upload hangs/freezes
- **Cause:** Network issue or server crashed
- **Fix:** Restart both server and client

### File corrupted after upload
- **Possible causes:**
  1. Two clients uploaded same filename (race condition)
  2. Upload interrupted mid-transfer
  3. Disk full on server
- **Fix:** Check `server/storage/` for partial files, delete and retry

---

## Summary Checklist

- [ ] Server builds successfully
- [ ] Client builds successfully
- [ ] Can register new user
- [ ] Can login with credentials
- [ ] Can upload small text file (< 1KB)
- [ ] Can upload binary file (1MB)
- [ ] Can upload large file (10MB+)
- [ ] File integrity verified (checksum matches)
- [ ] Error handling works (file not found)
- [ ] Server properly saves to storage/ folder

**If all checkboxes passed: Upload functionality is working! ✅**
