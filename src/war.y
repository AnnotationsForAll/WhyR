%token TOKEN_VAR TOKEN_VAR_EXT TOKEN_INT TOKEN_REAL TOKEN_WORD
%token TOKEN_TRUE TOKEN_FALSE TOKEN_FORALL TOKEN_EXISTS TOKEN_LET TOKEN_RESULT TOKEN_ZEXT TOKEN_SEXT TOKEN_NULL TOKEN_MIN TOKEN_MAX TOKEN_BADDR TOKEN_OP_IN TOKEN_OLD
%token TOKEN_TYPE_LLVM_INT TOKEN_TYPE_INT TOKEN_TYPE_BOOL TOKEN_TYPE_REAL TOKEN_TYPE_LLVM_FLOAT TOKEN_TYPE_LLVM_DOUBLE TOKEN_TYPE_STRUCT TOKEN_TYPE_VECTOR TOKEN_TYPE_SET
%token TOKEN_OP_IMP TOKEN_OP_BIDIR_IMP TOKEN_OP_AND TOKEN_OP_OR TOKEN_OP_EQ TOKEN_OP_NEQ TOKEN_OP_GE TOKEN_OP_LE
%token TOKEN_OP_SDIV TOKEN_OP_UDIV TOKEN_OP_REM TOKEN_OP_SREM TOKEN_OP_UREM TOKEN_OP_MOD TOKEN_OP_SMOD TOKEN_OP_UMOD TOKEN_OP_LSHL TOKEN_OP_LSHR TOKEN_OP_ASHR
%token TOKEN_OP_UGT TOKEN_OP_UGE TOKEN_OP_ULT TOKEN_OP_ULE TOKEN_OP_SGT TOKEN_OP_SGE TOKEN_OP_SLT TOKEN_OP_SLE
%token TOKEN_OP_FOEQ TOKEN_OP_FOGT TOKEN_OP_FOGE TOKEN_OP_FOLT TOKEN_OP_FOLE TOKEN_OP_FONE TOKEN_OP_FORD TOKEN_OP_FUEQ TOKEN_OP_FUGT TOKEN_OP_FUGE TOKEN_OP_FULT TOKEN_OP_FULE TOKEN_OP_FUNE TOKEN_OP_FUNO
%token TOKEN_PACKED_STRUCT_BEGIN TOKEN_PACKED_STRUCT_END
%token PREC_CAST PREC_QUANT PREC_LET PREC_UMINUS PREC_REF PREC_DEREF

%left ','
%right PREC_QUANT
%right '?' ':' PREC_LET
%right TOKEN_OP_IMP TOKEN_OP_BIDIR_IMP
%left TOKEN_OP_OR
%left TOKEN_OP_AND
%left '|'
%left '^'
%left '&'
%left TOKEN_OP_EQ TOKEN_OP_NEQ TOKEN_OP_IN
%left '<' '>' TOKEN_OP_LE TOKEN_OP_GE TOKEN_OP_UGT TOKEN_OP_UGE TOKEN_OP_ULT TOKEN_OP_ULE TOKEN_OP_SGT TOKEN_OP_SGE TOKEN_OP_SLT TOKEN_OP_SLE TOKEN_OP_FOEQ TOKEN_OP_FOGT TOKEN_OP_FOGE TOKEN_OP_FOLT TOKEN_OP_FOLE TOKEN_OP_FONE TOKEN_OP_FORD TOKEN_OP_FUEQ TOKEN_OP_FUGT TOKEN_OP_FUGE TOKEN_OP_FULT TOKEN_OP_FULE TOKEN_OP_FUNE TOKEN_OP_FUNO
%left TOKEN_OP_LSHL TOKEN_OP_LSHR TOKEN_OP_ASHR
%left '+' '-'
%left '*' '/' TOKEN_OP_SDIV TOKEN_OP_UDIV TOKEN_OP_REM TOKEN_OP_SREM TOKEN_OP_UREM TOKEN_OP_MOD TOKEN_OP_SMOD TOKEN_OP_UMOD
%right PREC_CAST '!' '~' PREC_UMINUS PREC_REF PREC_DEREF TOKEN_OLD
%left '[' ']' '{' '}'
%%
top_level:
      expr                                          { warParserReturnValue = $1; }
    | TOKEN_WORD ':' expr                           { warParserSource->label = $1.token; warParserReturnValue = $3; }
    ;
int_const:
      TOKEN_INT                                     { $$ = war_parse_int($1); }
    ;
real_const:
      TOKEN_REAL                                    { $$ = war_parse_real($1); }
    ;
bool_const:
      TOKEN_TRUE                                    { $$ = war_parse_bool_const(true); }
    | TOKEN_FALSE                                   { $$ = war_parse_bool_const(false); }
    ;
array_item:
      expr                                          { $$ = war_init_array_item($1); }
    | array_item ',' expr                           { $$ = war_extend_array_item($1, $3); }
    ;
array_const:
      '{' array_item '}'                            { $$ = war_parse_array_const($2); }
    ;
var_name:
      TOKEN_VAR                                     { $$ = war_parse_var($1); }
    | TOKEN_VAR_EXT                                 { $$ = war_parse_var_ext($1); }
    ;
type_list:
      typeid                                        { $$ = war_init_array_item($1); }
    | type_list ',' typeid                          { $$ = war_extend_array_item($1, $3); }
    ;
typeid:
      TOKEN_TYPE_LLVM_INT                           { $$ = war_parse_llvm_int_type($1); }
    | TOKEN_TYPE_INT                                { $$ = war_parse_int_type(); }
    | TOKEN_TYPE_BOOL                               { $$ = war_parse_bool_type(); }
    | TOKEN_TYPE_REAL                               { $$ = war_parse_real_type(); }
    | TOKEN_TYPE_LLVM_FLOAT                         { $$ = war_parse_llvm_float_type(); }
    | TOKEN_TYPE_LLVM_DOUBLE                        { $$ = war_parse_llvm_double_type(); }
    | typeid '*'                                    { $$ = war_parse_llvm_ptr_type($1); }
    | typeid '[' TOKEN_INT ']'                      { $$ = war_parse_llvm_array_type($1, $3); }
    | typeid '[' ']'                                { $$ = war_parse_llvm_array_0_type($1); }
    | TOKEN_TYPE_STRUCT TOKEN_WORD                  { $$ = war_parse_struct_type($2); }
    | TOKEN_TYPE_STRUCT '{' type_list '}'           { $$ = war_parse_anon_struct_type(false, $3); }
    | TOKEN_TYPE_STRUCT TOKEN_PACKED_STRUCT_BEGIN type_list TOKEN_PACKED_STRUCT_END
                                                    { $$ = war_parse_anon_struct_type(true , $3); }
    | typeid '[' TOKEN_INT ']' TOKEN_TYPE_VECTOR    { $$ = war_parse_vector_type($1, $3); }
    | typeid TOKEN_TYPE_SET                         { $$ = war_parse_set_type($2); }
    ;
cast:
      '(' typeid ')' expr                           %prec PREC_CAST
                                                    { $$ = war_parse_cast($2, $4); }
    | '(' typeid TOKEN_ZEXT ')' expr                %prec PREC_CAST
                                                    { $$ = war_parse_cast_ext(true , $2, $5); }
    | '(' typeid TOKEN_SEXT ')' expr                %prec PREC_CAST
                                                    { $$ = war_parse_cast_ext(false, $2, $5); }
    | '(' typeid ')' TOKEN_NULL                     %prec PREC_CAST
                                                    { $$ = war_parse_cast_null($2); }
    | '(' typeid ')' '{' '}'                        %prec PREC_CAST
                                                    { $$ = war_parse_cast_empty_array($2); }
    | '(' typeid TOKEN_ZEXT ')' TOKEN_MIN           %prec PREC_CAST
                                                    { $$ = war_parse_spec_llvm_const(true , '<', $2); }
    | '(' typeid TOKEN_ZEXT ')' TOKEN_MAX           %prec PREC_CAST
                                                    { $$ = war_parse_spec_llvm_const(true , '>', $2); }
    | '(' typeid TOKEN_SEXT ')' TOKEN_MIN           %prec PREC_CAST
                                                    { $$ = war_parse_spec_llvm_const(false, '<', $2); }
    | '(' typeid TOKEN_SEXT ')' TOKEN_MAX           %prec PREC_CAST
                                                    { $$ = war_parse_spec_llvm_const(false, '>', $2); }
    | '(' typeid ')' TOKEN_TYPE_STRUCT '{' array_item '}'
                                                    %prec PREC_CAST
                                                    { $$ = war_parse_struct_const($2, $6); }
    | '(' TOKEN_TYPE_VECTOR ')' '{' array_item '}'  %prec PREC_CAST
                                                    { $$ = war_parse_vector_const($5); }
    | '(' TOKEN_TYPE_SET ')' '{' array_item '}'     %prec PREC_CAST
                                                    { $$ = war_parse_set_const($5); }
    ;
declare_item:
        typeid TOKEN_VAR                            { $$ = war_parse_decl_item($1, $2); }
      | typeid TOKEN_VAR_EXT                        { $$ = war_parse_decl_item_ext($1, $2); }
      ;
declare_list:
      declare_item                                  { $$ = war_init_decl_list($1); }
    | declare_list ',' declare_item                 { $$ = war_extend_decl_list($1, $3); }
    ;
quant_token:
      TOKEN_FORALL                                  {  $$.quant.kind = true ; }
    | TOKEN_EXISTS                                  {  $$.quant.kind = false; }
    ;
quant_header:
      quant_token declare_list ';'                  { $$ = war_add_quant_locals($1, $2); }
    ;
quant:
      quant_header expr                             %prec PREC_QUANT
                                                    { $$ = war_parse_quant($1, $2); }
    ;
define_item:
      declare_item '=' expr                         { $$ = war_parse_def_item($1, $3); }
    ;
define_list:
      define_item                                   { $$ = war_init_def_list($1); }
    | define_list ',' define_item                   { $$ = war_extend_def_list($1, $3); }
    ;
let_header:
      TOKEN_LET define_list ';'                     { $$ = war_add_let_locals($2); }
    ;
let:
      let_header expr                               %prec PREC_LET
                                                    { $$ = war_parse_let($1, $2); }
    ;
expr:
      int_const
    | real_const
    | bool_const
    | array_const
    | var_name
    | TOKEN_RESULT                                  { $$ = war_parse_result(); }
    | expr '?' expr ':' expr                        { $$ = war_parse_ifte($1, $3, $5); }
    | expr TOKEN_OP_IMP expr                        { $$ = war_parse_bin_bool_op('i', $1, $3); }
    | expr TOKEN_OP_BIDIR_IMP expr                  { $$ = war_parse_bin_bool_op('b', $1, $3); }
    | expr TOKEN_OP_AND expr                        { $$ = war_parse_bin_bool_op('&', $1, $3); }
    | expr TOKEN_OP_OR expr                         { $$ = war_parse_bin_bool_op('|', $1, $3); }
    | expr '|' expr                                 { $$ = war_parse_bin_bits_op('|', $1, $3); }
    | expr '^' expr                                 { $$ = war_parse_bin_bits_op('^', $1, $3); }
    | expr '&' expr                                 { $$ = war_parse_bin_bits_op('&', $1, $3); }
    | expr TOKEN_OP_EQ expr                         { $$ = war_parse_eq_op(false, $1, $3); }
    | expr TOKEN_OP_NEQ expr                        { $$ = war_parse_eq_op(true , $1, $3); }
    | expr TOKEN_OP_IN expr                         { $$ = war_parse_in_op($1, $3); }
    | expr '<' expr                                 { $$ = war_parse_bin_comp_op('<', $1, $3); }
    | expr TOKEN_OP_LE expr                         { $$ = war_parse_bin_comp_op(',', $1, $3); }
    | expr '>' expr                                 { $$ = war_parse_bin_comp_op('>', $1, $3); }
    | expr TOKEN_OP_GE expr                         { $$ = war_parse_bin_comp_op('.', $1, $3); }
    | expr TOKEN_OP_ULT expr                        { $$ = war_parse_bin_comp_llvm_op(false, '<', $1, $3); }
    | expr TOKEN_OP_ULE expr                        { $$ = war_parse_bin_comp_llvm_op(false, ',', $1, $3); }
    | expr TOKEN_OP_UGT expr                        { $$ = war_parse_bin_comp_llvm_op(false, '>', $1, $3); }
    | expr TOKEN_OP_UGE expr                        { $$ = war_parse_bin_comp_llvm_op(false, '.', $1, $3); }
    | expr TOKEN_OP_SLT expr                        { $$ = war_parse_bin_comp_llvm_op(true , '<', $1, $3); }
    | expr TOKEN_OP_SLE expr                        { $$ = war_parse_bin_comp_llvm_op(true , ',', $1, $3); }
    | expr TOKEN_OP_SGT expr                        { $$ = war_parse_bin_comp_llvm_op(true , '>', $1, $3); }
    | expr TOKEN_OP_SGE expr                        { $$ = war_parse_bin_comp_llvm_op(true , '.', $1, $3); }
    | expr TOKEN_OP_FOEQ expr                       { $$ = war_parse_bin_comp_float_op(true , '=', $1, $3); }
    | expr TOKEN_OP_FOGT expr                       { $$ = war_parse_bin_comp_float_op(true , '>', $1, $3); }
    | expr TOKEN_OP_FOGE expr                       { $$ = war_parse_bin_comp_float_op(true , '.', $1, $3); }
    | expr TOKEN_OP_FOLT expr                       { $$ = war_parse_bin_comp_float_op(true , '<', $1, $3); }
    | expr TOKEN_OP_FOLE expr                       { $$ = war_parse_bin_comp_float_op(true , ',', $1, $3); }
    | expr TOKEN_OP_FONE expr                       { $$ = war_parse_bin_comp_float_op(true , '+', $1, $3); }
    | expr TOKEN_OP_FORD expr                       { $$ = war_parse_bin_comp_float_op(true , '!', $1, $3); }
    | expr TOKEN_OP_FUEQ expr                       { $$ = war_parse_bin_comp_float_op(false, '=', $1, $3); }
    | expr TOKEN_OP_FUGT expr                       { $$ = war_parse_bin_comp_float_op(false, '>', $1, $3); }
    | expr TOKEN_OP_FUGE expr                       { $$ = war_parse_bin_comp_float_op(false, '.', $1, $3); }
    | expr TOKEN_OP_FULT expr                       { $$ = war_parse_bin_comp_float_op(false, '<', $1, $3); }
    | expr TOKEN_OP_FULE expr                       { $$ = war_parse_bin_comp_float_op(false, ',', $1, $3); }
    | expr TOKEN_OP_FUNE expr                       { $$ = war_parse_bin_comp_float_op(false, '+', $1, $3); }
    | expr TOKEN_OP_FUNO expr                       { $$ = war_parse_bin_comp_float_op(false, '!', $1, $3); }
    | expr TOKEN_OP_LSHL expr                       { $$ = war_parse_shift_op('<', $1, $3); }
    | expr TOKEN_OP_LSHR expr                       { $$ = war_parse_shift_op('>', $1, $3); }
    | expr TOKEN_OP_ASHR expr                       { $$ = war_parse_shift_op('?', $1, $3); }
    | expr '+' expr                                 { $$ = war_parse_bin_math_op('+', $1, $3); }
    | expr '-' expr                                 { $$ = war_parse_bin_math_op('-', $1, $3); }
    | expr '*' expr                                 { $$ = war_parse_bin_math_op('*', $1, $3); }
    | expr '/' expr                                 { $$ = war_parse_bin_math_op('/', $1, $3); }
    | expr TOKEN_OP_SDIV expr                       { $$ = war_parse_bin_bits_op('/', $1, $3); }
    | expr TOKEN_OP_UDIV expr                       { $$ = war_parse_bin_bits_op('?', $1, $3); }
    | expr TOKEN_OP_MOD expr                        { $$ = war_parse_bin_math_op('%', $1, $3); }
    | expr TOKEN_OP_SMOD expr                       { $$ = war_parse_bin_bits_op('%', $1, $3); }
    | expr TOKEN_OP_UMOD expr                       { $$ = war_parse_bin_bits_op('5', $1, $3); }
    | expr TOKEN_OP_REM expr                        { $$ = war_parse_bin_math_op('$', $1, $3); }
    | expr TOKEN_OP_SREM expr                       { $$ = war_parse_bin_bits_op('$', $1, $3); }
    | expr TOKEN_OP_UREM expr                       { $$ = war_parse_bin_bits_op('4', $1, $3); }
    | '~' expr                                      { $$ = war_parse_bit_not($2); }
    | '!' expr                                      { $$ = war_parse_not($2); }
    | '-' expr                                      %prec PREC_UMINUS
                                                    { $$ = war_parse_neg($2); }
    | '*' expr                                      %prec PREC_DEREF
                                                    { $$ = war_parse_load($2); }
    | '&' expr                                      %prec PREC_REF
                                                    { $$ = war_parse_ref($2); }
    | TOKEN_OLD expr                                { $$ = war_parse_old($2); }
    | TOKEN_BADDR '(' expr ',' TOKEN_VAR ')'        { $$ = war_parse_baddr($3, $5); }
    | TOKEN_BADDR '(' expr ',' TOKEN_VAR_EXT ')'    { $$ = war_parse_baddr_ext($3, $5); }
    | expr '[' expr ']'                             { $$ = war_parse_index($1, $3); }
    | expr '[' expr '=' expr ']'                    { $$ = war_parse_set_index($1, $3, $5); }
    | cast
    | quant
    | let
    | '(' expr ')'                                  { $$ = $2; }
    ;
%%
