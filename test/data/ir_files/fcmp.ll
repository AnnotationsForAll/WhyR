define i1 @test_false(double %x, double %y) !whyr.ensures !{!{!"war", !"! (bool) result"}} {
    %a = fcmp false double %x, %y
    ret i1 %a
}

define i1 @test_oeq(double %x, double %y) !whyr.ensures !{!{!"eq", !{!"bool", !{!"result"}}, !{!"foeq", !{!"arg", !"x"}, !{!"arg", !"y"}}}} {
    %a = fcmp oeq double %x, %y
    ret i1 %a
}

define i1 @test_ule(double %x, double %y) !whyr.ensures !{!{!"eq", !{!"bool", !{!"result"}}, !{!"fule", !{!"arg", !"x"}, !{!"arg", !"y"}}}} {
    %a = fcmp ule double %x, %y
    ret i1 %a
}

define i1 @test_ord(double %x, double %y) !whyr.ensures !{!{!"eq", !{!"bool", !{!"result"}}, !{!"ford", !{!"arg", !"x"}, !{!"arg", !"y"}}}} {
    %a = fcmp ord double %x, %y
    ret i1 %a
}

define i1 @test_uno(double %x, double %y) !whyr.ensures !{!{!"eq", !{!"bool", !{!"result"}}, !{!"funo", !{!"arg", !"x"}, !{!"arg", !"y"}}}} {
    %a = fcmp uno double %x, %y
    ret i1 %a
}

define i1 @test_true(double %x, double %y) !whyr.ensures !{!{!"war", !"(bool) result"}} {
    %a = fcmp true double %x, %y
    ret i1 %a
}
