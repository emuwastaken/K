/% -------------------------------------------------
stress_goto_label_loop.k
Exercises: ETIKETT, GÅ TILL, OM/ANNARS, comparisons, ÖKAR MED
-------------------------------------------------%/

HEL: ENTRE()<
    HEL: i, 0;

    ETIKETT START;

    i ÖKAR MED 1;

    OM (i STÖRRE 10)<
        GÅ TILL DONE;
    >

    GÅ TILL START;

    ETIKETT DONE;

    ÅTERVÄND i;
>
