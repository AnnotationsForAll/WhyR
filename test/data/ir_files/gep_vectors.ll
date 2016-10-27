@list = global < 4 x i32 > < i32 1, i32 2, i32 3, i32 4 >

define i32* @gep() !whyr.ensures !{!{!"war", !"*result == (i32)4"}} {
    %a = getelementptr < 4 x i32 >, < 4 x i32 >* @list, i32 0, i32 3
    ret i32* %a
}

define <2 x i32*> @vgep() {
    %a = getelementptr < 4 x i32 >, < 4 x i32 >* @list, <2 x i32> <i32 0, i32 0>, <2 x i32> <i32 3, i32 3>
    ret <2 x i32*> %a
}
