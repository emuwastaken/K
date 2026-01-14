

TOM BERÄKNA(HEL PEK: a, HEL PEK: b)<
    VÄRDE VID a: VÄRDE VID a + VÄRDE VID b;
    VÄRDE VID b: VÄRDE VID a - VÄRDE VID b;
    ÅTERVÄND;
>

HEL ENTRE()<

    HEL: x, 5;
    HEL: y, 8;

    BERÄKNA(ADRESS AV x, ADRESS AV y)

    ÅTERVÄND x+y;
>