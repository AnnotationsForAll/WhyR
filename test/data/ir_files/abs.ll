define i32 @abs(i32 %n) {
    %is_neg = icmp slt i32 %n, 0
    %neg = sub nsw i32 0, %n
    %val = select i1 %is_neg, i32 %neg, i32 %n
    ret i32 %val
}
