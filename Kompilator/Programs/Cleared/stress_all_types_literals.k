/% -------------------------------------------------
stress_all_types_literals.k
Exercises: HEL/FLYT/BOK/BIT/HALV/BYTE/ORD/VAL/TOM, string and float literals, OCH/ELLER/INTE, parentheses
-------------------------------------------------%/

TOM: NOP()<
    ÅTERVÄND;
>

HEL: ENTRE()<
    HEL: i, 10;
    FLYT: f, 3.5;

    /% strings (BOK is char; pointers use BOK PEK) %/
    BOK PEK: s, "hej";

    BIT: flag, 1;
    HALV: h, 2;
    BYTE: by, 255;
    ORD: w, 12345;
    VAL: v, 42;

    /% logical stress %/
    OM ((i STÖRRE 5) OCH INTE (i MINDRE 0) ELLER flag)<
        i ÖKAR MED 1;
    > ANNARS <
        i MINSKAR MED 1;
    >

    /% function call statement %/
    NOP();

    ÅTERVÄND i + h + by + w + v;
>
