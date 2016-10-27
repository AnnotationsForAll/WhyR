define i32 @test_select(i1 %cond) !whyr.ensures !{!{!"war", !"(bool)%cond ==> result == (i32)2 && !(bool)%cond ==> result == (i32)4"}} {
    %a = select i1 %cond, i32 2, i32 4
    ret i32 %a
}

define <2 x i32> @test_select_vector(<2 x i1> %cond) {
    %a = select <2 x i1> %cond, <2 x i32> <i32 2, i32 3>, <2 x i32> <i32 1, i32 4>
    ret <2 x i32> %a
}
