define i1 @test_eq(i32 %x, i32 %y) !whyr.requires !{!{!"war", !"%x == (i32)2 && %y == (i32)2"}} !whyr.ensures !{!{!"war", !"(bool)result"}} {
    %a = icmp eq i32 %x, %y
    ret i1 %a
}

define i1 @test_neq(i32 %x, i32 %y) !whyr.requires !{!{!"war", !"%x == (i32)2 && %y == (i32)2"}} !whyr.ensures !{!{!"war", !"!(bool)result"}} {
    %a = icmp ne i32 %x, %y
    ret i1 %a
}

define i1 @test_sgt(i32 %x, i32 %y) !whyr.requires !{!{!"war", !"%x == (i32)2 && %y == (i32)2"}} !whyr.ensures !{!{!"war", !"!(bool)result"}} {
    %a = icmp sgt i32 %x, %y
    ret i1 %a
}

define i1 @test_slt(i32 %x, i32 %y) !whyr.requires !{!{!"war", !"%x == (i32)2 && %y == (i32)2"}} !whyr.ensures !{!{!"war", !"!(bool)result"}} {
    %a = icmp slt i32 %x, %y
    ret i1 %a
}

define i1 @test_sge(i32 %x, i32 %y) !whyr.requires !{!{!"war", !"%x == (i32)2 && %y == (i32)2"}} !whyr.ensures !{!{!"war", !"(bool)result"}} {
    %a = icmp sge i32 %x, %y
    ret i1 %a
}

define i1 @test_sle(i32 %x, i32 %y) !whyr.requires !{!{!"war", !"%x == (i32)2 && %y == (i32)2"}} !whyr.ensures !{!{!"war", !"(bool)result"}} {
    %a = icmp sle i32 %x, %y
    ret i1 %a
}

define i1 @test_ugt(i32 %x, i32 %y) !whyr.requires !{!{!"war", !"%x == (i32)2 && %y == (i32)2"}} !whyr.ensures !{!{!"war", !"!(bool)result"}} {
    %a = icmp ugt i32 %x, %y
    ret i1 %a
}

define i1 @test_ult(i32 %x, i32 %y) !whyr.requires !{!{!"war", !"%x == (i32)2 && %y == (i32)2"}} !whyr.ensures !{!{!"war", !"!(bool)result"}} {
    %a = icmp ult i32 %x, %y
    ret i1 %a
}

define i1 @test_uge(i32 %x, i32 %y) !whyr.requires !{!{!"war", !"%x == (i32)2 && %y == (i32)2"}} !whyr.ensures !{!{!"war", !"(bool)result"}} {
    %a = icmp uge i32 %x, %y
    ret i1 %a
}

define i1 @test_ule(i32 %x, i32 %y) !whyr.requires !{!{!"war", !"%x == (i32)2 && %y == (i32)2"}} !whyr.ensures !{!{!"war", !"(bool)result"}} {
    %a = icmp ule i32 %x, %y
    ret i1 %a
}