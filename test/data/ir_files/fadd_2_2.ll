define double @add_pos() !whyr.ensures !{!{!"eq", !{!"real", !"4.0"}, !{!"add", !{!"real", !"2"}, !{!"real", !"2"}}}} {
    %a = fadd double 2.0, 2.0
    ret double %a
}

define double @add_neg() !whyr.ensures !{!{!"eq", !{!"real", !"-4.0"}, !{!"add", !{!"real", !"-2"}, !{!"real", !"-2"}}}} {
    %a = fadd double -2.0, -2.0
    ret double %a
}
