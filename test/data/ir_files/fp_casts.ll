define float @test_fptrunc() !whyr.ensures !{!{!"eq", !{!"result"}, !{!"real.to.float", !{!"typeof", float 0.0}, !{!"real", !"2.5"}}}} {
    %a = fptrunc double 2.5 to float
    ret float %a
}

define double @test_fpext() !whyr.ensures !{!{!"eq", !{!"result"}, !{!"real.to.float", !{!"typeof", double 0.0}, !{!"real", !"2.5"}}}} {
    %a = fpext float 2.5 to double
    ret double %a
}

define i32 @test_fptoui() {
    %a = fptoui double 2.5 to i32
    ret i32 %a
}

define i32 @test_fptosi() {
    %a = fptosi double 2.5 to i32
    ret i32 %a
}

define double @test_uitofp() {
    %a = uitofp i32 2 to double
    ret double %a
}

define double @test_sitofp() {
    %a = sitofp i32 2 to double
    ret double %a
}