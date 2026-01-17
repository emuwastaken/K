/% -------------------------------------------------
stress_enum_switch_goto.k
Exercises: ENUM, VÄXEL/FALL/ANNARS, GÅ TILL, ETIKETT, OM/ANNARS, comparisons
-------------------------------------------------%/

ENUM FARG <
    ROD: 1,
    GRON: 2,
    BLA: 3
>;

HEL: ENTRE()<
    FARG: c, ROD;
    HEL: score, 0;

    /% goto/label smoke %/
    GÅ TILL START;

    ETIKETT START;

    /% switch over enum value (no BRYT used) %/
    VÄXEL(c)<
        FALL ROD:
            score: 10;
            c: GRON;

        FALL GRON:
            score ÖKAR MED 5;
            c: BLA;

        FALL BLA:
            score ÖKAR MED 20;

        ANNARS:
            score: 0;
    >

    OM (score STÖRRE 30)<
        GÅ TILL DONE;
    > ANNARS <
        score ÖKAR MED 1;
    >

    ETIKETT DONE;

    ÅTERVÄND score;
>
