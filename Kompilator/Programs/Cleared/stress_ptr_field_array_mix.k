/% -------------------------------------------------
stress_ptr_field_array_mix.k
Exercises: STRUKTUR, FÄLT, arrays <>, array access, VÄRDE VID, ADRESS AV, %
-------------------------------------------------%/

STRUKTUR NODE <
    HEL: V;
>

HEL: ENTRE()<
    HEL: a, 1;
    HEL PEK: p, ADRESS AV a;

    /% write then read through pointer %/
    VÄRDE VID p: 2;
    HEL: b, VÄRDE VID p;

    /% array declaration and element assignment %/
    HEL<4>: xs;
    xs<0>: 10;
    xs<1>: 11;
    xs<2>: 12;
    xs<3>: 13;

    STRUKTUR NODE: n;

    /% field assignment from expression using array + modulo %/
    FÄLT n V: b + (xs<2> % 5);

    ÅTERVÄND FÄLT n V;
>
