define i32 @four_if_true(i1 %cond) !whyr.requires !{!{!"eq", !{!"arg", !"cond"}, i1 true}} !whyr.ensures !{!{!"eq", !{!"result"}, i32 4}} {
    br i1 %cond, label %IfTrue, label %IfFalse
    IfTrue:
    ret i32 4
    IfFalse:
    ret i32 2
}

define i32 @two_if_false(i1 %cond) !whyr.requires !{!{!"eq", !{!"arg", !"cond"}, i1 false}} !whyr.ensures !{!{!"eq", !{!"result"}, i32 2}} {
    br i1 %cond, label %IfTrue, label %IfFalse
    IfTrue:
    ret i32 4
    IfFalse:
    ret i32 2
}

define i32 @four_if_true_no_requires(i1 %cond) !whyr.ensures !{!{!"->", !{!"bool", !{!"arg", !"cond"}}, !{!"eq", !{!"result"}, i32 4}}} {
    br i1 %cond, label %IfTrue, label %IfFalse
    IfTrue:
    ret i32 4
    IfFalse:
    ret i32 2
}

define i32 @two_if_false_no_requires(i1 %cond) !whyr.ensures !{!{!"->", !{!"not", !{!"bool", !{!"arg", !"cond"}}}, !{!"eq", !{!"result"}, i32 2}}} {
    br i1 %cond, label %IfTrue, label %IfFalse
    IfTrue:
    ret i32 4
    IfFalse:
    ret i32 2
}

define i32 @conditional_ret(i1 %cond) !whyr.ensures !{!{!"and", !{!"->", !{!"not", !{!"bool", !{!"arg", !"cond"}}}, !{!"eq", !{!"result"}, i32 2}}, !{!"->", !{!"bool", !{!"arg", !"cond"}}, !{!"eq", !{!"result"}, i32 4}}}} {
    br i1 %cond, label %IfTrue, label %IfFalse
    IfTrue:
    ret i32 4
    IfFalse:
    ret i32 2
}