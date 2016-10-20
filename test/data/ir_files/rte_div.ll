define i32 @rte(i32 %x, i32 %y) {
    %a = sdiv i32 %x, %y
    %b = udiv exact i32 %x, %y
    %c = srem i32 %a, %b
    %d = urem i32 %c, %c
    ret i32 %d
}
