/% -------------------------------------------------
call_args_complex_lvalues.k
-------------------------------------------------%/

STRUKTUR PERSON <
    HEL: SCORE;
>

TOM: PUT(HEL PEK: out, HEL: v)<
    VÄRDE VID out: v;
    ÅTERVÄND;
>

HEL: ENTRE()<
    STRUKTUR PERSON: p;
    HEL<4>: arr, <1, 2, 3, 4>;

    /% pass address-of field access as argument %/
    PUT(ADRESS AV (FÄLT p SCORE), (1 + 2) * 3);

    /% pass address-of array element as argument %/
    PUT(ADRESS AV (arr<2>), 99);

    /% read back through pointers to stress unary/primary mixing %/
    HEL PEK: pa, ADRESS AV (arr<2>);
    HEL: x, (VÄRDE VID pa) + (FÄLT p SCORE);

    ÅTERVÄND x;
>
