/% -------------------------------------------------
quicksort.k
-------------------------------------------------%/

HEL<10>: arr, <5, 1, 4, 2, 8, 0, 2, 9, 3, 7>;

TOM: BYT(HEL: i, HEL: j)<
    HEL: tmp, arr<i>;
    arr<i>: arr<j>;
    arr<j>: tmp;
    ÅTERVÄND;
>

HEL: PARTITION(HEL: low, HEL: high)<
    HEL: pivot, arr<high>;
    HEL: i, low;
    HEL: j, low;

    MEDAN(j MINDRE high)<
        OM(arr<j> MINDRE pivot)<
            BYT(i, j);
            i ÖKAR;
        >
        j ÖKAR;
    >

    BYT(i, high);
    ÅTERVÄND i;
>

TOM: QUICKSORT(HEL: low, HEL: high)<
    OM(low MINDRE high)<
        HEL: p, PARTITION(low, high);

        OM(low MINDRE p)<
            QUICKSORT(low, p - 1);
        >

        OM((p + 1) MINDRE (high + 1))<
            QUICKSORT(p + 1, high);
        >
    >

    ÅTERVÄND;
>

HEL: ENTRE()<
    QUICKSORT(0, 9);
    ÅTERVÄND arr<0> + arr<9>;
>
