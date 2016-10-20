define i32 @ifte_outside(i1 %cond) !whyr.ensures !{!{!"ifte", !{!"eq", !{!"arg", !"cond"}, i1 1}, !{!"eq", !{!"result"}, i32 4}, !{!"eq", !{!"result"}, i32 2}}} {
    br i1 %cond, label %IfTrue, label %IfFalse
    IfTrue:
    ret i32 4
    IfFalse:
    ret i32 2
}

define i32 @ifte_inside(i1 %cond) !whyr.ensures !{!{!"eq", !{!"result"}, !{!"ifte", !{!"eq", !{!"arg", !"cond"}, i1 1}, i32 4, i32 2}}} {
    br i1 %cond, label %IfTrue, label %IfFalse
    IfTrue:
    ret i32 4
    IfFalse:
    ret i32 2
}