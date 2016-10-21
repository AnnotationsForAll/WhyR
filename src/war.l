%%
true                                { return TOKEN_TRUE; }
false                               { return TOKEN_FALSE; }
forall                              { return TOKEN_FORALL; }
exists                              { return TOKEN_EXISTS; }
let                                 { return TOKEN_LET; }
result                              { return TOKEN_RESULT; }
zext                                { return TOKEN_ZEXT; }
sext                                { return TOKEN_SEXT; }
null                                { return TOKEN_NULL; }
min                                 { return TOKEN_MIN; }
max                                 { return TOKEN_MAX; }
blockaddress                        { return TOKEN_BADDR; }
sdiv                                { return TOKEN_OP_SDIV; }
udiv                                { return TOKEN_OP_UDIV; }
rem                                 { return TOKEN_OP_REM; }
srem                                { return TOKEN_OP_SREM; }
urem                                { return TOKEN_OP_UREM; }
mod                                 { return TOKEN_OP_MOD; }
smod                                { return TOKEN_OP_SMOD; }
umod                                { return TOKEN_OP_UMOD; }
shl                                 { return TOKEN_OP_LSHL; }
lshr                                { return TOKEN_OP_LSHR; }
ashr                                { return TOKEN_OP_ASHR; }
ugt                                 { return TOKEN_OP_UGT; }
uge                                 { return TOKEN_OP_UGE; }
ult                                 { return TOKEN_OP_ULT; }
ule                                 { return TOKEN_OP_ULE; }
sgt                                 { return TOKEN_OP_SGT; }
sge                                 { return TOKEN_OP_SGE; }
slt                                 { return TOKEN_OP_SLT; }
sle                                 { return TOKEN_OP_SLE; }
foeq                                { return TOKEN_OP_FOEQ; }
fogt                                { return TOKEN_OP_FOGT; }
foge                                { return TOKEN_OP_FOGE; }
folt                                { return TOKEN_OP_FOLT; }
fole                                { return TOKEN_OP_FOLE; }
fone                                { return TOKEN_OP_FONE; }
ford                                { return TOKEN_OP_FORD; }
fueq                                { return TOKEN_OP_FUEQ; }
fugt                                { return TOKEN_OP_FUGT; }
fuge                                { return TOKEN_OP_FUGE; }
fult                                { return TOKEN_OP_FULT; }
fule                                { return TOKEN_OP_FULE; }
fune                                { return TOKEN_OP_FUNE; }
funo                                { return TOKEN_OP_FUNO; }
int                                 { return TOKEN_TYPE_INT; }
bool                                { return TOKEN_TYPE_BOOL; }
real                                { return TOKEN_TYPE_REAL; }
float                               { return TOKEN_TYPE_LLVM_FLOAT; }
double                              { return TOKEN_TYPE_LLVM_DOUBLE; }
struct                              { return TOKEN_TYPE_STRUCT; }
i[0-9]+                             {
                                        yylval.token = yytext+1;
                                        return TOKEN_TYPE_LLVM_INT;
                                    }
[0-9]*\.[0-9]+                      {
                                        yylval.token = yytext;
                                        return TOKEN_REAL;
                                    }
[0-9]+                              {
                                        yylval.token = yytext;
                                        return TOKEN_INT;
                                    }
[$%@]\"[^\"]*\"                     {
                                        yylval.token = yytext;
                                        return TOKEN_VAR_EXT;
                                    }
[$%@][-a-zA-Z$._][-a-zA-Z$._0-9]*   {
                                        yylval.token = yytext;
                                        return TOKEN_VAR;
                                    }
"==>"                               { return TOKEN_OP_IMP; }
"<==>"                              { return TOKEN_OP_BIDIR_IMP; }
"&&"                                { return TOKEN_OP_AND; }
"||"                                { return TOKEN_OP_OR; }
"=="                                { return TOKEN_OP_EQ; }
"!="                                { return TOKEN_OP_NEQ; }
">="                                { return TOKEN_OP_GE; }
"<="                                { return TOKEN_OP_LE; }
"<{"                                { return TOKEN_PACKED_STRUCT_BEGIN; }
"}>"                                { return TOKEN_PACKED_STRUCT_END; }
[a-zA-Z_][a-zA-Z0-9_]*              {
                                        yylval.token = yytext;
                                        return TOKEN_WORD;
                                    }
[-()+*/=<>&\|!~^?:\.\[\]{},;]       { return *yytext; }
[\n\t\v ]+                          // ignore whitespace
.                                   {
                                        char* s = strdup("invalid character ' '");
                                        s[19] = *yytext;
                                        yyerror(s);
                                    }
%%
