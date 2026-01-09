typedef enum TokenType {

    //Structure
    TOK_PROGRAM = 200,
    TOK_FORFATTARE,
    TOK_ENTRE,

    //Types
    TOK_HEL,            //int
    TOK_FLYT,           //float
    TOK_BOK,            //char

    TOK_BIT,            // bit value
    TOK_HALV,           // 4 bit value
    TOK_BYTE,           // 8 bit value
    TOK_ORD,            // 16 bit value

    TOK_VAL,            //enum
    TOK_STRUKTUR,       //struct
    TOK_FALT,           // composite value and derefernce access to structs

    TOK_TOM,            //void

    //Qualifiers
    TOK_PEK,            // *
    TOK_KONSTANT,       //const
    TOK_STATISK,        //static

    //Selection and Repetition
    TOK_OM,
    TOK_ANNARS,
    TOK_MEDAN,
    TOK_GOR,
    TOK_FOR,
    TOK_VAXEL,
    TOK_FALL,
    TOK_BRYT,
    TOK_FORTSATT,

    //Comparison and Boolean operations
    TOK_EQ,         // LIKA            (==)
    TOK_NEQ,        // INTE LIKA       (!=)
    TOK_LT,         // MINDRE ÄN       (<)
    TOK_GT,         // STÖRRE ÄN       (>)
    TOK_LTE,        // MINLIK          (<=)
    TOK_GTE,        // STÖLIK          (>=)

    TOK_OCH,        // OCH             (&&)
    TOK_ELLER,      // ELLER           (||)
    TOK_INTE,       // INTE             (!)

    //Flow / Return
    TOK_ATERVAND,   //ÅTERVÄND         (return)        

    //Operators
    TOK_ASSIGN,     //  :
    TOK_PLUS,       //  +
    TOK_MINUS,      //  -
    TOK_MUL,        //  *
    TOK_DIV,        //  /
    TOK_EXP,        //  ^

    TOK_OKAR,           //  ÖKAR        (++ equivalent)
    TOK_MINSKAR,        //  MINSKAR     (-- equivalent)
    TOK_PLUS_ASSIGN,    //  ÖKAR MED    (+= equivalent)
    TOK_MINUS_ASSIGN,   //  MINSKAR MED (-= equivalent)
    TOK_MUL_ASSIGN,     //  MULT MED    (*= equivalent)
    TOK_DIV_ASSIGN,     //  DELAS MED   (/= equivalent)

    TOK_DEREF,          //  #   dereference
    TOK_ADDRESS,        //  £   address-of
    TOK_STORLEKAV,       // STORLEK AV  (keyword baserad sizeof())


    //Delimiters
    TOK_LPAREN,     //  (
    TOK_RPAREN,     //  )
    TOK_LBLOCK,     //  <
    TOK_RBLOCK,     //  >
    TOK_SEMI,       //  ;
    TOK_COMMA,      //  ,

    //Literals and Identifiers
    TOK_IDENTIFIER,
    TOK_INT_LIT,
    TOK_FLOAT_LIT,


    //Misc. and Special
    TOK_EOF,
    TOK_ERROR

} TokenType;


typedef struct Keyword {
    const char * lexeme;
    TokenType token; 
} Keyword;

static const Keyword keywords[] = {

    /* Structure */
    { "PROGRAM",     TOK_PROGRAM        },
    { "FÖRFATTARE",  TOK_FORFATTARE     },
    { "ENTRE",       TOK_ENTRE          },

    /* Types */
    { "HEL",         TOK_HEL            },
    { "FLYT",        TOK_FLYT           },
    { "BOK",         TOK_BOK            },

    { "BIT",         TOK_BIT            },
    { "HALV",        TOK_HALV           },
    { "BYTE",        TOK_BYTE           },
    { "ORD",         TOK_ORD            },

    { "VAL",         TOK_VAL            },
    { "STRUKTUR",    TOK_STRUKTUR       },
    { "FÄLT",        TOK_FALT           },

    { "TOM",         TOK_TOM            },

    /* Qualifiers */
    { "PEK",         TOK_PEK            },
    { "KONSTANT",    TOK_KONSTANT       },
    { "STATISK",     TOK_STATISK        },

    /* Selection and repetition */
    { "OM",          TOK_OM             },
    { "ANNARS",      TOK_ANNARS         },
    { "MEDAN",       TOK_MEDAN          },
    { "GÖR",         TOK_GOR            },
    { "FÖR",         TOK_FOR            },

    { "VÄXEL",       TOK_VAXEL          },
    { "FALL",        TOK_FALL           },
    { "BRYT",        TOK_BRYT           },
    { "FORTSATT",    TOK_FORTSATT       },

    /* Boolean / logical */
    { "OCH",         TOK_OCH            },
    { "ELLER",       TOK_ELLER          },
    { "INTE",        TOK_INTE           },

    /* Flow */
    { "ÅTERVÄND",    TOK_ATERVAND       },

    /* Special operators */
    { "ÖKAR",        TOK_OKAR           },
    { "ÖKAR MED",    TOK_PLUS_ASSIGN    },
    { "MINSKAR",     TOK_MINSKAR        },
    { "MINSKAR MED", TOK_MINUS_ASSIGN   },
    { "MULT MED",    TOK_MUL_ASSIGN     },
    { "DELAS MED",   TOK_DIV_ASSIGN     },

    { "VÄRDE VID",   TOK_DEREF          },
    { "ADRESS AV",   TOK_ADDRESS        },
    { "STORLEK AV",  TOK_STORLEKAV      },

    /* Sentinel */
    { NULL,          TOK_ERROR          }
};


TokenType lookup_pair   (char * lexeme, char * lextwo);
TokenType lookup        (char * lexeme);