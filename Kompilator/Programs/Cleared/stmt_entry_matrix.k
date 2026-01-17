/% -------------------------------------------------
stmt_entry_matrix.k
-------------------------------------------------%/

STRUKTUR PERSON <
    HEL: SCORE;
>

TOM: TOUCH(HEL PEK: p)<
    VÄRDE VID p ÖKAR MED 1;
    ÅTERVÄND;
>

HEL: ENTRE()<
    HEL: a, 1;
    HEL PEK: p, ADRESS AV a;

    /% deref assignment as statement starter %/
    VÄRDE VID p: 2;

    /% deref compound update as statement starter %/
    VÄRDE VID p ÖKAR MED 3;
    VÄRDE VID p MINSKAR MED 1;

    /% struct-typed declaration as statement starter %/
    STRUKTUR PERSON: s;

    /% field assignment as statement starter %/
    FÄLT s SCORE: 10;

    /% field compound update as statement starter %/
    FÄLT s SCORE ÖKAR MED 5;
    FÄLT s SCORE MINSKAR MED 2;

    /% identifier-start expression statement (function call) %/
    TOUCH(p);

    ÅTERVÄND (VÄRDE VID p) + (FÄLT s SCORE);
>
