define i32 @f(i32 %x) !whyr.requires !{!{!"eq", !{!"arg", !"x"}, i32 4}} {
    %a = add i32 2, 2
    ret i32 %a
}

define i32 @main() !whyr.ensures !{!{!"eq", !{!"result"}, i32 4}} {
    %b = call i32 @f(i32 4)
    ret i32 %b
}
