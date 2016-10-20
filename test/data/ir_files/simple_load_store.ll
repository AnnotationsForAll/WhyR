define i32 @main(i32* %p) !whyr.ensures !{!{!"eq", !{!"result"}, i32 4}} {
    store i32 4, i32* %p
    %a = load i32, i32* %p
    ret i32 %a
}