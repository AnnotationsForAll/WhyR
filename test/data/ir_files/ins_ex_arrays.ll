define i32 @ex_simple() !whyr.ensures !{!{!"war", !"result == (i32)2"}} {
    %a = extractvalue [ 4 x i32 ] [ i32 1, i32 2, i32 3, i32 4 ], 1
    ret i32 %a
}

define i32 @ex_nest() !whyr.ensures !{!{!"war", !"result == (i32)1"}} {
    %a = extractvalue [ 2 x [ 2 x [ 2 x i32 ] ] ] [ [ 2 x [ 2 x i32 ] ] [ [ 2 x i32 ] [ i32 1, i32 2 ], [ 2 x i32 ] [ i32 1, i32 2 ] ], [ 2 x [ 2 x i32 ] ] [ [ 2 x i32 ] [ i32 1, i32 2 ], [ 2 x i32 ] [ i32 1, i32 2 ] ] ], 0, 1, 0
    ret i32 %a
}

define [ 4 x i32 ] @ins_simple() !whyr.ensures !{!{!"war", !"result == {(i32)1, (i32)42, (i32)3, (i32)4}"}} {
    %a = insertvalue [ 4 x i32 ] [ i32 1, i32 2, i32 3, i32 4 ], i32 42, 1
    ret [ 4 x i32 ] %a
}

define [ 2 x [ 2 x [ 2 x i32 ] ] ] @ins_nest() !whyr.ensures !{!{!"war", !"result[0][1][0] == (i32)42"}} {
    %a = insertvalue [ 2 x [ 2 x [ 2 x i32 ] ] ] [ [ 2 x [ 2 x i32 ] ] [ [ 2 x i32 ] [ i32 1, i32 2 ], [ 2 x i32 ] [ i32 1, i32 2 ] ], [ 2 x [ 2 x i32 ] ] [ [ 2 x i32 ] [ i32 1, i32 2 ], [ 2 x i32 ] [ i32 1, i32 2 ] ] ], i32 42, 0, 1, 0
    ret [ 2 x [ 2 x [ 2 x i32 ] ] ] %a
}
