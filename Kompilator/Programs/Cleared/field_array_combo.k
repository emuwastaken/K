/% -------------------------------------------------
field_array_combo.k
-------------------------------------------------%/

STRUKTUR PERSON <
    HEL: ÅLDER;
    HEL: SCORE;
>

TOM: SET_FIELD(HEL PEK: out, HEL: v)<
    VÄRDE VID out: v;
    ÅTERVÄND;
>

HEL: ENTRE()<
    /% array of structs %/
    STRUKTUR PERSON<3>: ps;

    /% initialize fields via FÄLT on array elements %/
    FÄLT ps<0> ÅLDER: 18;
    FÄLT ps<0> SCORE: 10;

    FÄLT ps<1> ÅLDER: 25;
    FÄLT ps<1> SCORE: 20;

    /% take address of a field and write through pointer %/
    HEL PEK: sp, ADRESS AV (FÄLT ps<1> SCORE);
    VÄRDE VID sp ÖKAR MED 5;

    /% out-parameter write to a different field %/
    SET_FIELD(ADRESS AV (FÄLT ps<2> SCORE), 99);

    ÅTERVÄND (FÄLT ps<0> SCORE) + (FÄLT ps<1> SCORE) + (FÄLT ps<2> SCORE);
>
