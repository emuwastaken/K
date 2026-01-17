/% -------------------------------------------------
type_spec_combo.k
-------------------------------------------------%/

STRUKTUR PERSON <
    HEL: SCORE;
>

HEL: ENTRE()<
    /% struct type specifier variations %/
    STRUKTUR PERSON: p;
    STRUKTUR PERSON PEK: pp;

    /% pointer + array dimensions on built-in types %/
    HEL PEK<3>: ps;

    /% plain arrays and pointers %/
    HEL<2>: xs, <11, 22>;
    HEL PEK: px, ADRESS AV xs<0>;

    /% mix deref and field access in expressions %/
    FÄLT p SCORE: 7;
    VÄRDE VID px ÖKAR MED (FÄLT p SCORE);

    ÅTERVÄND (VÄRDE VID px) + (FÄLT p SCORE);
>
