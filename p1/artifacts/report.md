### Bug 1: Decode doesn't check if a buffer is allocated or not
- Category: No initialization check
- Trigger: We can give decode an input whre buf is null. For instance
```
uint8 *buf;
size_t used = decode_record(buf, wrote, &out); // will segfault
```
- Root cause: The decode funciton does not check whether or not `buf` exists, so when it tries to access it in the line `uint8_t name_len = buf[off++];` this will cause a segmentation fault.
- Fix: we can check before we access the buf area whether or not it is allocated through `if (!buf) return 0;`

### Bug 2: Encode doesn't check whether score count matches the length of the score array passed in
- Category: Out of bound read/copy
- Trigger: We can give a score_count that is bigger than the actual amount of scores there are like:
```
in.score_count = 10;
uint16_t tmp_scores[2] = {4,10};
```
This will cause the encoded object to decode into:
```
=== Decoded Record ===
Name   : alice
Age    : 23
Scores : [4, 10, 0, 0, 0, 0, 27745, 25449, 101, 0]
```
which can leak whatever's on the stack past the array.
- Root cause: The encode function does not check whether the length of of the `scores` array actually matches `score_count` before copying them onto the output buffer
- Fix: we can check if the length of the `scores` array mathces `score_count` first before copying it else, we return an error.

`if(rec->score_count != 0 && rec->score_count != (sizeof(rec->scores)/sizeof(rec->scores[0])))`

### Bug 3: Encode doesn't check whether the input record is allcoated or not
- Category: No initialization check
- Trigger: We can pass in a NULL pointer into encode and it will cause a segfault
- Rootcause: The encode funciton does not check whether or not `rec` exists, so when it tries to access it in the line `strlen(rec->name);` it will cause a segmentation fault.
- Fix: we can check before we access the buf area whether or not it is allocated through `if (!rec) return 0;`

### Bug 4: The `age` and `score_count` fields are vulnerable to integer overflows
- Category: Integer Overflow
- Trigger: We can input an age and a score count that is beyond the capacity of `uint8` and `uint32` such as 
```
    in.age = 10234;
    in.score_count = 999999999999999999999999999999999;
```
If we kept the array of scores to the default length of 3, it will cause the print function to read too many addresses, eventually leading to an illegal address and a segfault.
- Rootcause: since the program defines the types of these 2 fields as `uint8` and `uint32`, when a number bigger than the 8 or 32 bit gets input into the struct, only the least significant 8/32 will be kept. This sometimes lead to values that don't make sense such as limiting an age to be less than 150 or having a hard 1000 cap to the number of scores.
- Fix: I will implement a hard limit to these variables and return -1 if they go beyond it. This will reduce the risk of segfaulting since we're less likely to be reading too far in areas of memory that we don't have access to.


- Rootcause: The types for `age` and `score_count` 
### Bug 5: Free record leaves a dangling pointer
- Category: Dangling pointer
- Trigger: We can call `free_record()` once and then again on a record pointer which will then cause a double free error
- Rootcause: `free_record()` does not set the pointer it receives to null after freeing it. This pointer left dangling can potentially cause future crashes if used after the free.
- Fix: we can make the method take in a double pointer to a struct and then modify the pointer's value to be NULL after we've called free

### Bug 6: Encode does not check for how big the buffer size is
- Category: Buffer overflow
- Trigger: we can allocate a buffer that is smaller than our struct to pass into to `encode_record()`. This does not inherently trigger a crash but can cause memory corruption, potentially leading to a crash.
- Rootcause: `encode_record()` relies on the trust that the user will allocate a buffer of approriate size themselves, which can be taken advantage of. By allocating a buffer smaller than what we actually need, we can write garbage into an area of memory beyond the buffer.
- Fix: Instead of making the user allocate a buffer themselves, we can make the function allocate and mutate the buffer for them. This prevents the user from providing any buffer that may be too small to be copied to.