/% -------------------------------------------------
expr_precedence_stress.k
-------------------------------------------------%/

HEL: F(HEL: x, HEL: y)<
    ÅTERVÄND (x * 3) + (y / 2) - 7;
>

HEL: ENTRE()<
    HEL: a, 5;
    HEL: b, 2;
    HEL: c, 10;
    HEL: d, 3;

    HEL: e, 0;

    /% precedence: unary > mul/div > add/sub > relational > logical %/
    e:  INTE (a MINDRE b) OCH ( (a + b * c - d) MINDRE (F(c, d) + (a * (b + 1))) );

    /% deeper nesting %/
    e: e OCH ( INTE ( (a + b) MINDRE (c - d) ) );

    /% mixed arithmetic with parentheses and unary minus %/
    HEL: r, (a + (b * (c - d))) - (-(a * b)) + (+c);

    ÅTERVÄND r;
>
