/% -------------------------------------------------
binary_search.k
-------------------------------------------------%/

/% Sorted ascending %/
HEL<10>: arr, <0, 1, 2, 3, 4, 5, 7, 8, 9, 10>;

HEL: BINARSOK(HEL: target)<
    HEL: low, 0;
    HEL: high, 9;

    MEDAN(low MINDRE (high + 1))<
        HEL: mid, (low + high) / 2;

        /% arr[mid] < target %/
        OM(arr<mid> MINDRE target)<
            low: mid + 1;
        >

        /% target < arr[mid] %/
        OM(target MINDRE arr<mid>)<
            high: mid - 1;
        >

        /% otherwise: neither is smaller => equal %/
        OM( (INTE (arr<mid> MINDRE target)) OCH (INTE (target MINDRE arr<mid>)) )<
            ÅTERVÄND mid;
        >
    >

    ÅTERVÄND -1;
>

HEL: ENTRE()<
    /% Change target to test both found and not-found %/
    HEL: target, 8;
    HEL: idx, BINARSOK(target);

    ÅTERVÄND idx;
>
