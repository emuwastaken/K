/% -------------------------------------------------
return_pointer_forms.k
-------------------------------------------------%/

HEL PEK: GETPTR(HEL PEK: p)<
    ÅTERVÄND p;
>

HEL: GETVAL(HEL PEK: p)<
    ÅTERVÄND VÄRDE VID p;
>

HEL: INCVAL(HEL PEK: p)<
    ÅTERVÄND (VÄRDE VID p) + 1;
>

HEL PEK: ADDROF_LOCAL()<
    HEL: a, 7;
    ÅTERVÄND ADRESS AV a;
>

HEL: ENTRE()<
    HEL: x, 5;
    HEL PEK: px, ADRESS AV x;

    HEL PEK: q, GETPTR(px);
    HEL: a, GETVAL(q);
    HEL: b, INCVAL(q);

    /% parse-only return form %/
    HEL PEK: r, ADDROF_LOCAL();

    ÅTERVÄND a + b + (VÄRDE VID px);
>
