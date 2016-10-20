define i32 @main(i32 %a) !whyr.requires !{!{!"eq", !{!"arg", !"a"}, i32 1}} !whyr.ensures !{!{!"eq", !{!"result"}, i32 4}} {
    BlockOne:
    %0 = add i32 %a, %a
    br label %BlockTwo
    BlockTwo:
    %1 = add i32 %0, %0
    br label %BlockThree
    BlockThree:
    ret i32 %1
}