/% -------------------------------------------------
deref_chain_precedence.k
-------------------------------------------------%/

HEL: ENTRE()<
    HEL: a, 1;

    /% pointer and pointer-to-pointer %/
    HEL PEK: p, ADRESS AV a;
    HEL PEK PEK: pp, ADRESS AV p;

    /% precedence stress with nested deref %/
    HEL: x, VÄRDE VID (VÄRDE VID pp) + 1;
    HEL: y, (VÄRDE VID (VÄRDE VID pp)) * 2;

    /% write through nested deref %/
    VÄRDE VID (VÄRDE VID pp): 77;

    /% compound update through nested deref %/
    VÄRDE VID (VÄRDE VID pp) ÖKAR MED 5;
    VÄRDE VID (VÄRDE VID pp) MINSKAR MED 3;

    ÅTERVÄND a + x + y;
>
