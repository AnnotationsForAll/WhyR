define i32 @f(i32* %p) {
    store i32 4, i32* %p
    %a = load i32, i32* %p
    ret i32 %a
}

define i32 @main(i32* %q) !whyr.ensures !{!{!"eq", !{!"result"}, i32 4}} {
    %1 = call i32 @f(i32* %q)
    ret i32 %1
}