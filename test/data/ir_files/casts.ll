define i2 @test_trunc() !whyr.ensures !{!{!"eq", !{!"result"}, i2 2}} {
    %a = trunc i4 14 to i2
    ret i2 %a
}

define i2 @test_trunc_2() !whyr.ensures !{!{!"eq", !{!"result"}, i2 2}} {
    %a = trunc i4 2 to i2
    ret i2 %a
}

define i64 @test_zext(i32 %x) !whyr.requires !{!{!"war", !"%x == (i32)1230"}} !whyr.ensures !{!{!"war", !"result == (i64)1230"}} {
    %a = zext i32 %x to i64
    ret i64 %a
}

define i64 @test_sext(i32 %x) !whyr.requires !{!{!"war", !"%x == -(i32)1230"}} !whyr.ensures !{!{!"war", !"result == -(i64)1230"}} {
    %a = sext i32 %x to i64
    ret i64 %a
}

define i2 @test_trunc_exact(i4 %x) !whyr.ensures !{!{!"eq", !{!"result"}, !{!"trunc", !{!"typeof", i2 0}, !{!"arg", !"x"}}}} {
    %a = trunc i4 %x to i2
    ret i2 %a
}

define i64 @test_zext_exact(i32 %x) !whyr.ensures !{!{!"eq", !{!"result"}, !{!"sext", !{!"typeof", i64 0}, !{!"arg", !"x"}}}}  {
    %a = zext i32 %x to i64
    ret i64 %a
}

define i64 @test_sext_exact(i32 %x) !whyr.ensures !{!{!"eq", !{!"result"}, !{!"zext", !{!"typeof", i64 0}, !{!"arg", !"x"}}}}  {
    %a = sext i32 %x to i64
    ret i64 %a
}
