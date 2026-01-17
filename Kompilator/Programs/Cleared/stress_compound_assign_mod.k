/% -------------------------------------------------
stress_compound_assign_mod.k
Exercises: ':', ÖKAR, MINSKAR, ÖKAR MED, MINSKAR MED, MULT MED, DELAS MED, %
Note: MULT MED / DELAS MED may require statement routing updates to treat them as assignment/update statements.
-------------------------------------------------%/

HEL: ENTRE()<
    HEL: a, 20;

    /% plain assignment %/
    a: a + 1;

    /% postfix updates %/
    a ÖKAR;
    a MINSKAR;

    /% compound updates %/
    a ÖKAR MED 3;
    a MINSKAR MED 2;

    /% compound assignment operators (C-style *= and /= analogs) %/
    a MULT MED 4;
    a DELAS MED 2;

    /% modulo %/
    a: a % 6;

    ÅTERVÄND a;
>
