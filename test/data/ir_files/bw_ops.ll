define i32 @test_and() !whyr.ensures !{!{!"eq", !{!"result"}, i32 0}} {
    %a = and i32 2, 0
    ret i32 %a
}

define i32 @test_or() !whyr.ensures !{!{!"eq", !{!"result"}, i32 2}} {
    %a = or i32 2, 0
    ret i32 %a
}

define i32 @test_shl() !whyr.ensures !{!{!"eq", !{!"result"}, i32 8}} {
    %a = shl i32 2, 2
    ret i32 %a
}

define i32 @test_lshr() !whyr.ensures !{!{!"eq", !{!"result"}, i32 0}} {
    %a = lshr i32 2, 2
    ret i32 %a
}