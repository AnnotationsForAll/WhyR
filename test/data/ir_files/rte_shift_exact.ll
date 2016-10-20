define i32 @rte(i32 %x, i32 %y) {
    %a = shl nuw i32 %x, %y
    %b = ashr exact i32 %x, %y
    %c = lshr exact i32 %a, %b
    %d = shl nuw nsw i32 %c, 2
    ret i32 %d
}
