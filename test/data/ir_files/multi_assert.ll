define i32 @f(i32 %x) !whyr.ensures !{!{!"war", !"result == %x * (i32)4"}} {
    %a = add i32 %x, %x
    %b = add i32 %a, %a, !whyr.assert !{!{!"war", !"%a == %x * (i32)2"}}
    ret i32 %b, !whyr.assert !{!{!"war", !"%b == %a * (i32)2"}}
}
