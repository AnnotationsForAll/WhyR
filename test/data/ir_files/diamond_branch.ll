define i32 @f(i1 %cond) !whyr.ensures !{!{!"war", !"(bool) %cond ? result == (i32)4 : result == (i32)8"}} {
    %a = alloca i32
    br i1 %cond, label %if_t, label %if_f
    if_t:
    %four = add i32 2,2
    store i32 %four, i32* %a
    br label %final
    if_f:
    %eight = add i32 4,4
    store i32 %eight, i32* %a
    br label %final
    final:
    %f = load i32, i32* %a
    ret i32 %f
}