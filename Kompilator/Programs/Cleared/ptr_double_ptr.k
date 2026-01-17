/% -------------------------------------------------
ptr_double_ptr.k
-------------------------------------------------%/

TOM: SWAP_PTR(HEL PEK PEK: a, HEL PEK PEK: b)<
    HEL PEK: tmp, VÄRDE VID a;
    VÄRDE VID a: VÄRDE VID b;
    VÄRDE VID b: tmp;
    ÅTERVÄND;
>

HEL: ENTRE()<
    HEL: x, 7;
    HEL: y, 13;

    HEL PEK: px, ADRESS AV x;
    HEL PEK: py, ADRESS AV y;

    /% swap via pointer-to-pointer %/
    SWAP_PTR(ADRESS AV px, ADRESS AV py);

    /% verify by reading through swapped pointers %/
    ÅTERVÄND (VÄRDE VID px) + (VÄRDE VID py);
>
