/% -------------------------------------------------
stress_bitwise_shift_mod.k
Exercises: BITOCH, BITELLER, BITXOR, BITINTE, SKIFT VÄNSTER/HÖGER, %, precedence
-------------------------------------------------%/

HEL: ENTRE()<
    HEL: a, 29;
    HEL: b, 14;

    HEL: andv, a BITOCH b;
    HEL: orv,  a BITELLER b;
    HEL: xorv, a BITXOR b;
    HEL: inv,  BITINTE a;

    HEL: shl, a SKIFT VÄNSTER 2;
    HEL: shr, a SKIFT HÖGER 1;

    HEL: modv, a % 5;

    /% mixed precedence: shifts bind tighter than bitwise here (parser dependent) %/
    HEL: combo, (a SKIFT VÄNSTER 1) BITOCH (b SKIFT HÖGER 1);

    /% shift compound updates (statement-level) %/
    a SKIFT VÄNSTER MED 1;
    a SKIFT HÖGER MED 2;

    /% ordinary update %/
    a ÖKAR MED 3;

    ÅTERVÄND andv + orv + xorv + inv + shl + shr + modv + combo + a;
>
