/% -------------------------------------------------
assign_update_stress.k
-------------------------------------------------%/

HEL<6>: arr, <10, 20, 30, 40, 50, 60>;

STRUKTUR BOX<
    HEL: X;
    HEL: Y;
>

TOM: STRESS()<
    HEL: i, 0;
    BOX: b;

    b: b; /% keep if your grammar requires initialization patterns; otherwise remove %/

    FÄLT b X: 1;
    FÄLT b Y: 2;

    /% plain assignments %/
    i: 2;
    arr<i>: 99;

    /% postfix updates %/
    i ÖKAR;
    i MINSKAR;
    arr<i> ÖKAR;
    arr<i> MINSKAR;

    /% “ÖKAR MED / MINSKAR MED” (compound) %/
    i ÖKAR MED 5;
    i MINSKAR MED 3;

    arr<i> ÖKAR MED (10 + 1);
    arr<i> MINSKAR MED (2 * 3);

    FÄLT b X ÖKAR MED 7;
    FÄLT b Y MINSKAR MED 4;

    /% mix with expressions on RHS for assignment %/
    FÄLT b X: (FÄLT b X) + arr<i> * 2;
    arr<i>: arr<i> + (FÄLT b Y);

    ÅTERVÄND;
>

HEL: ENTRE()<
    STRESS();
    ÅTERVÄND arr<0> + arr<5>;
>
