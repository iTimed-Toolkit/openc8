# Known bugs

## Payloads

- Payloads must contain at least one explicit `PAYLOAD_SECTION` in order to compile successfully.
- Need a better way to call BootROM functions from assembly --- currently need to pass pointers through an intermediate function in C.

## IO

- Big reads / writes might need to be chunked into smaller ones depending on max r/w sizes.