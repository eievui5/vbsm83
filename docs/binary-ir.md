The back end currently reads IR in its binary form, which is a collection of
strings, declarations, and statements.

# Strings

The binary begins with a large list of strings, which are re-used by the
contents of the file to avoid redundancy. These strings each begin with an
unsigned 64-bit integer denoting their length, and the strings end when a length
of 0 is read.

# Declarations

After the strings, the file then contains a list of declarations. Declarations
are structured as follows:

```
struct {
    uint8_t storage_class;
    const char* identifier;
    const char** traits;
    uint8_t type;
    bool is_fn;
};
```

First, an 8-bit value is read, which is interpreted as the declaration's storage
class. These values are as follows:

```
STATIC = 0
EXTERN = 1
EXPORT = 2
```

Any other values are considered invalid and should cause an error.

Then, a single string is used as the declaration's identifier. This is denoted
by a 64-bit value which is used to index the master list of strings.

Following this index is a flexible list of additional strings which make up the
trait list. This list is again made up of 64-bit indices, and is terminated by a
-1 (0xFFFFFFFFFFFFFFFF).

Next is a single 8-bit value which denotes the type of the declaration. These
types are:

```
VOID =  0
U8   =  1
U16  =  2
U32  =  3
U64  =  4
I8   =  5
I16  =  6
I32  =  7
I64  =  8
F32  =  9
F64  = 10
PTR  = 11
```

Finally, the declaration ends with a single 8-bit value. This value should be
set to 0 if a variable is being declared, or 1 if a function is being declared.

However, if the declaration is either STATIC or EXPORT (The first byte was
either 1 or 2) and the declaration is marked as a function, it should be
followed by a list of statements.

# Statements

Statements are the most complex part of the binary format and have many
different representations.

All statements begin with an 8-bit value which determines what kind of statement
they are.

```
OPERATION =   0
READ      =   1
WRITE     =   2
JUMP      =   3
RETURN    =   4
LABEL     =   5
END_BLOCK = 255
```

The value 255 is special. It denotes the end of a block of statements, at which
point the program expects to read declarations from the file again.

## OPERATION

The first type of statement is an OPERATION.

```
struct Operation {
    // Begins with a 0
    uint8_t type;
    uint64_t dest;
    uint64_t lhs;
    bool is_const;
    uint64_t rhs;
```

The first byte following the 0 contains the operation type. This is one of the
following values:

```
// Assignment operator
ASSIGN     =  0
// Binary operators
ADD        =  1
SUB        =  2
MUL        =  3
DIV        =  4
MOD        =  5
B_AND      =  6
B_OR       =  7
B_XOR      =  8
L_AND      =  9
L_OR       = 10
LSH        = 11
RSH        = 12
LESS       = 13
GREATER    = 14
LESS_EQU   = 15
GREATER_EQ = 16
NOT_EQU    = 17
EQU        = 18
// Unary operators
NOT        = 19
NEGATE     = 20
COMPLEMENT = 21
```

Following this byte is a 64-bit integer which represents the index of the local
variable where this operation will be stored.

Next up is another 64-bit integer, which is the first of the operands in an
operation. This is always the index of another local variable. **This value is
ignored by ASSIGN.**

Following this value is a single byte, which is either 0 or 1. If 0, the
following number should be treated as the index of a local variable, the same as
the destination and lhs. However, if it is 1, it will be interpreted as a
constant value. ASSIGN uses this to determine whether it is assigning a constant
value to a variable, or performing a copy. **This value is ignored by NOT,
NEGATE, and COMPLEMENT.**

## READ and WRITE

These two statement types are represented by 1 and 2 respectively and are very
simple.

Following the statement type is a 64-bit value. This is the index of the local
variable which is either being read into, or written to a global.

Then the READ or WRITE ends with another 64-bit value, this time the index of a
string. This represents the identifier of the variable being read or written to.

## JUMP

JUMP is represented by a 3 and contains one value: a 64-bit string index. This
string is the identifier of the label being jumped to.

## RETURN

RETURN is represented by a 4.

RETURN begins with a single byte, which is either 0 or 1. If 0, the following
number should be treated as the index of a local variable. If it is 1, it will
be interpreted as a constant value.

## LABEL

LABEL is represented by a 5.

Like JUMP, it only contains one 64-bit value: the index of its identifier
string.
