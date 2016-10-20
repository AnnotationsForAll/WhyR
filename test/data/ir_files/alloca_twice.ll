define i32 @f(i32 %x) !whyr.requires !{!{!"war", !"result == (i32)4"}} {
    %p = alloca i32
    %q = alloca i32
    store i32 %x, i32* %p
    store i32 2, i32* %q
    %a = load i32, i32* %p
    ret i32 %a
}