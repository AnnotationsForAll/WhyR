define i32 @main() !whyr.requires !{!{!"true"}} !whyr.ensures !{!{!"eq", !{!"result"}, i32 4}} {
    %a = add i32 2, 2
    ret i32 %a
}