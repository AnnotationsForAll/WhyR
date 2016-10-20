@"hello world" = global i32 0

define i32 @"'''"(i32 %.a, i32 %b$c) !whyr.ensures !{!{!"eq", !{!"result"}, !{!"arg", !".a"}}} {
    %"_'_" = add i32 %.a, %b$c
    %"0" = sub i32 %"_'_", %"_'_"
    %1 = mul i32 %.a, %"0"
    %"0$" = add i32 %1, %b$c
    ret i32 %.a
}
