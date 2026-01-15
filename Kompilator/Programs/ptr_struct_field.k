/% -------------------------------------------------
ptr_struct_field.k
-------------------------------------------------%/

STRUKTUR PERSON <
    HEL: ÅLDER;
    HEL: SCORE;
>

TOM: SET_SCORE(HEL PEK: out, HEL: v)<
    VÄRDE VID out: v;
    ÅTERVÄND;
>

HEL: ENTRE()<
    STRUKTUR PERSON: p;

    /% take address of a field via FÄLT access %/
    HEL PEK: sp, ADRESS AV (FÄLT p SCORE);

    /% write via pointer %/
    VÄRDE VID sp: 42;

    /% out-parameter write %/
    SET_SCORE(sp, 99);

    ÅTERVÄND FÄLT p SCORE;
>
