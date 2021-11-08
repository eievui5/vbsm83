ABI propositions

## Parameters
- In order, `c`, `b`, `e`, `d`, `l`, `h`, `<stack>`
  - By putting the lower byte first, we can speed up casts from u8 to u16. While
    possibly niche, I see no analogus benefit to using the high byte.
- 8-bit values take up a single register or byte.
- 16-bit values take up two, but are *aligned* to the next register pair
  - For example `function(char byte, short word)` will store `byte` in `c`, and
    `word` in `de`, skipping `b`.
  - `function2(char byte1, short word, char byte2)` will store `byte1` in `c`,
    and `word` in `de`, then go *back* to `b` to store `byte2`.
  - This way values are packed efficiently while still being useful -- the ABI
    should prioritize function over simplicity.
- 32-bit values take up four bytes and are also aligned to register pairs.
  - The top and bottom halves of the value may be split freely among `bc`, `de`,
    and `hl`.
- 64-bit values immediately go to the stack, as there are only 48 bits between
the parameter registers.
- Aggregates will attempt to pack themselves into registers, but if *any*
  members do not fit, the entire structure must go to the stack.
  - This prevents a structure from being split between registers and the stack,
    which could prove especially problematic when a pointer is needed and the
    whole struct must move to the stack.
  - For example passing a `struct point {char x; char y;};` would act as if two
    chars had been passed, placing `x` into `c` and `y` into `b`.
  - However, `char name[16]` could not fit into just 6 registers, and so the
    entire array would be copied onto the stack.
- The accumulator is reserved. The compiler may use it for anything *but* a
  function parameter.
  - My rationale for this is that functions are unlikely to need the first
    parameter in `a` immediately. Since `a` is used for nearly every operation,
    passing a register in `a` would not be very useful. *The ABI should prioritize
    function over simplicity.*
  - Additionally, this frees up `a` to be used as an "internal parameter", say
    for a `farcall` function which must change the bank without corrupting the
    target function's arguments.