define i32 @main() !whyr.requires !{!{!"true"}} !whyr.ensures !{!{!"eq", !{!"result"}, !{!"add", i32 2, i32 2}}} {
    %a = add i32 2, 2
    ret i32 %a
}