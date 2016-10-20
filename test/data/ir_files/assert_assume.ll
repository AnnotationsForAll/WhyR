define i32 @main(i32 %x) {
    %a = add i32 %x, 2
    ret i32 %a, !whyr.assume !{!{!"eq", !{!"var", !"x"}, i32 2}}, !whyr.assert !{!{!"eq", !{!"var", !"a"}, i32 4}}
}