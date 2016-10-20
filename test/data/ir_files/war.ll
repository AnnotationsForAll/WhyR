define i32 @main(i32 %x) !whyr.requires !{!{!"war", !"%x == (i32)2"}} !whyr.ensures !{!{!"war", !"%x + (i32)2 == (i32)4"}} {
    %a = add i32 2, %x
    ret i32 %a
}