/% -------------------------------------------------
ptr_array_elem.k
-------------------------------------------------%/

HEL<5>: arr, <1, 2, 3, 4, 5>;

HEL: ENTRE()<
    HEL: i, 2;

    /% pointer to arr<i> %/
    HEL PEK: p, ADRESS AV arr<i>;

    /% precedence stress with deref %/
    HEL: a, VÄRDE VID p + 10;
    HEL: b, (VÄRDE VID p) * 2;

    /% write back to array element %/
    VÄRDE VID p: a + b;

    ÅTERVÄND arr<2>;
>
