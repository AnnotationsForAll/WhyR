define i32 @test_select(i1 %cond) !whyr.ensures !{!{!"war", !"(bool)%cond ==> result == (i32)2 && !(bool)%cond ==> result == (i32)4"}} {
    %a = select i1 %cond, i32 2, i32 4
    ret i32 %a
}