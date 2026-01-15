/% -------------------------------------------------
bubble_sort.k
-------------------------------------------------%/

HEL<10>: arr, <5, 1, 4, 2, 8, 0, 2, 9, 3, 7>;

TOM: BUBBLA()<
    HEL: n, 10;
    HEL: i, 0;

    MEDAN(i MINDRE n)<
        HEL: j, 0;
        HEL: gräns, (n - 1) - i;

        MEDAN(j MINDRE gräns)<
            OM(arr<(j + 1)> MINDRE arr<j>)<
                HEL: tmp, arr<j>;
                arr<j>: arr<(j + 1)>;
                arr<(j + 1)>: tmp;
            >

            j ÖKAR;
        >

        i ÖKAR;
    >

    ÅTERVÄND;
>

HEL: ENTRE()<
    BUBBLA();

    ÅTERVÄND arr<0> + arr<9>;
>
