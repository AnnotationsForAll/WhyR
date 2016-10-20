define i32 @switcher(i32 %x) !whyr.requires !{!{!"war", !"%x == (i32)0"}} !whyr.ensures !{!{!"war", !"result == (i32)4"}} {
    switch i32 %x, label %default [ i32 0, label %if_zero i32 1, label %if_one i32 400, label %if_four_hundred ]
    if_zero:
    ret i32 4
    if_one:
    ret i32 8
    if_four_hundred:
    ret i32 16
    default:
    ret i32 -1
}

define i32 @uncond_br(i32 %x) !whyr.ensures !{!{!"war", !"result == %x"}} {
    switch i32 %x, label %dest [  ]
    dest:
    ret i32 %x
}

define i32 @cond_br(i32 %x) !whyr.ensures !{!{!"war", !"result == (i32)(%x == (i32)0 ? 1 : 0)"}} {
    switch i32 %x, label %if_true [ i32 0, label %if_false ]
    if_true:
    ret i32 0
    if_false:
    ret i32 1
}
