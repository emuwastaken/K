PROGRAM basic_switch.k
FÖRFATTARE : Emu Nilsson

HEL ENTRE()
<
    HEL: a, 4;
    HEL: b, 5;

    VÄXEL(a)
    <
        FALL 3:
            a: a+b;
            BRYT;
    
        FALL 4:
            a: a-b;
            BRYT;

        ANNARS:
            b: a+b;
            BRYT;
    >

    ÅTERVÄND a+b;
>