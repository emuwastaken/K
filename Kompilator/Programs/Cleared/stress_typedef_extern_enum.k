/% -------------------------------------------------
stress_typedef_extern_enum.k
Exercises: TYPDEF, user-defined types, EXTERN on function, ENUM as type decl
-------------------------------------------------%/

TYPDEF HEL HELTAL;

ENUM MODE <
    MODE_A: 0,
    MODE_B: 1
>

EXTERN HELTAL: EXT_ADD(HELTAL: a, HELTAL: b)<
    ÅTERVÄND a + b;
>

HEL: ENTRE()<
    HELTAL: x, 3;
    HELTAL: y, 4;
    MODE: m, MODE_A;

    OM (m LIKA MODE_A)<
        x: EXT_ADD(x, y);
    > ANNARS <
        x: 0;
    >

    ÅTERVÄND x;
>
