define i8 @get_elem(i32 %n) !whyr.ensures !{!{!"war", !"%n == (i32)2 ==> result == (i8)4"}} {
    %a = extractelement <4 x i8> <i8 1, i8 2, i8 4, i8 8>, i32 %n
    ret i8 %a
}

define <4 x i8> @set_elem(i32 %n) {
    %a = insertelement <4 x i8> <i8 1, i8 2, i8 4, i8 8>, i8 0, i32 %n
    ret <4 x i8> %a
}
