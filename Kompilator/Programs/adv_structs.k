TYPDEF STRUKTUR person <
    HEL:        ålder;
    FLYT:       längd;
    person<2>:  föräldrar;          
>

HEL ENTRE()<

    HEL:  år, 2026
    BOK PEK:  månad, "Januari";
    BOK PEK:  maj, "Maj";

    person:     emu, <
        25, 193, <>
    >;

    person:     anne, <
        55, 165, <>
    >;

    person:     bengt, <
        58, 190, <>
    >

    FÄLT emu föräldrar<0>:  anne;
    FÄLT emu föräldrar<1>:  bengt;

    OM(månad LIKA maj)<
        FÄLT emu ålder ÖKAR;                // Samma som emu.ålder++;
        FÄLT bengt ålder ÖKAR MED 2         //Va fan farsan blev 2 år äldre
        FÄLT anne längd MINSKAR MED 4        //Morsan krympte
    > ANNARS <
        FÄLT emu ålder MINSKAR MED 24;    //Haha get turned into baby idiot
        FÄLT emu längd MINSKAR MED 140;    
    >

    ÅTERVÄND 0;

>