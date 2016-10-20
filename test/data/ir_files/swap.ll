define i32 @swap(i32* %p, i32* %q) !whyr.ensures !{!{!"war", !"%p == %q ==> result == (i32)6"}} {
    ; store initial values
    store i32 1, i32* %p
    store i32 3, i32* %q
    ; swap
    %vp = load i32, i32* %p
    %vq = load i32, i32* %q
    store i32 %vq, i32* %p
    store i32 %vp, i32* %q
    ; load and add
    %a = load i32, i32* %p
    %b = load i32, i32* %q
    %c = add i32 %a, %b
    ret i32 %c
}

define i32 @swap_alloca() !whyr.ensures !{!{!"war", !"result == (i32)4"}} {
    %p = alloca i32
    %q = alloca i32
    ; store initial values
    store i32 1, i32* %p
    store i32 3, i32* %q
    ; swap
    %vp = load i32, i32* %p
    %vq = load i32, i32* %q
    store i32 %vq, i32* %p
    store i32 %vp, i32* %q
    ; load and add
    %a = load i32, i32* %p
    %b = load i32, i32* %q
    %c = add i32 %a, %b
    ret i32 %c
}
