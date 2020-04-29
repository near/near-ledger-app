# Gotchas

## Speculos emulator can be useful, but for now fakes keys

## Beware of unaligned RAM access

```
        uint16_t *ptr16 = &tmp_ctx.signing_context.buffer[processed]; 
        PRINTF("uint16_t: %d", ptr16[0]);
```

`ptr16[0]` is crashing, even though `tmp_ctx.signing_context.buffer[processed]` (`unsigned char*`) can be accessed alright.


## PIC hackery

See https://ledger.readthedocs.io/en/latest/userspace/memory.html#pic-and-model-implications
