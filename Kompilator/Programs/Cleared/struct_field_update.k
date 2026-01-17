/% -------------------------------------------------
struct_field_update.k
-------------------------------------------------%/

STRUKTUR PERSON<
    HEL: ÅLDER;
    HEL: POÄNG;
>

TOM: FYLL(PERSON: p)<
    FÄLT p ÅLDER: 20;
    FÄLT p POÄNG: 100;

    FÄLT p ÅLDER ÖKAR;
    FÄLT p POÄNG MINSKAR;

    FÄLT p ÅLDER ÖKAR MED 5;
    FÄLT p POÄNG MINSKAR MED 40;

    ÅTERVÄND;
>

HEL: ENTRE()<
    PERSON: p;

    FYLL(p);

    ÅTERVÄND (FÄLT p ÅLDER) + (FÄLT p POÄNG);
>
