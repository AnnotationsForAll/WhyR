define i32 @rte(i32 %x, i32 %y) {
    %a = add nuw i32 %x, %y
    %b = sub nsw i32 %x, %y
    %c = mul nuw nsw i32 %a, %b
    ret i32 %c
}
