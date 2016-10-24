%simple = type { float, i32, i16* }
%nest = type { %simple, i64, i1, %simple }
%mixed = type { i32, [ 3 x [2 x %simple ] ], i32 }

define i32 @ex_simple() !whyr.ensures !{!{!"war", !"result == (i32)2"}} {
    %a = extractvalue %simple {float 1.0, i32 2, i16* null}, 1
    ret i32 %a
}

define i32 @ex_nest() !whyr.ensures !{!{!"war", !"result == (i32)2"}} {
    %a = extractvalue %nest {%simple {float 1.0, i32 2, i16* null},i64 1000, i1 true, %simple {float -1.0, i32 4, i16* null}}, 0, 1
    ret i32 %a
}

define i32 @ex_mixed() !whyr.ensures !{!{!"war", !"result == (i32)2"}} {
    %a = extractvalue %mixed {i32 42, [ 3 x [2 x %simple ] ] [[2 x %simple ] [%simple {float 1.0, i32 -1000, i16* null}, %simple {float 1.0, i32 420, i16* null}], [2 x %simple ] [%simple {float 1.0, i32 566, i16* null}, %simple {float 1.0, i32 0, i16* null}], [2 x %simple ] [%simple {float 1.0, i32 -12, i16* null}, %simple {float 1.0, i32 2, i16* null}]], i32 24}, 1, 2, 1, 1
    ret i32 %a
}

define %simple @ins_simple() !whyr.ensures !{!{!"war", !"result == (struct simple) struct {(float)1.0,(i32)2,(i16*)null}"}} {
    %a = insertvalue %simple {float 1.0, i32 45, i16* null}, i32 2, 1
    ret %simple %a
}

define %nest @ins_nest() !whyr.ensures !{!{!"war", !"result[0][1] == (i32)2"}} {
    %a = insertvalue %nest {%simple {float 1.0, i32 99, i16* null},i64 1000, i1 true, %simple {float -1.0, i32 4, i16* null}}, i32 2, 0, 1
    ret %nest %a
}

define %mixed @ins_mixed() !whyr.ensures !{!{!"war", !"result[1][2][1][1] == (i32)2"}} {
    %a = insertvalue %mixed {i32 42, [ 3 x [2 x %simple ] ] [[2 x %simple ] [%simple {float 1.0, i32 -1000, i16* null}, %simple {float 1.0, i32 420, i16* null}], [2 x %simple ] [%simple {float 1.0, i32 566, i16* null}, %simple {float 1.0, i32 0, i16* null}], [2 x %simple ] [%simple {float 1.0, i32 -12, i16* null}, %simple {float 1.0, i32 777, i16* null}]], i32 24}, i32 2, 1, 2, 1, 1
    ret %mixed %a
}
