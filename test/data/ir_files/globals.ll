@x = global i32 1
@y = global i32 3

define i32 @main() !whyr.requires !{!{!"true"}} !whyr.ensures !{!{!"eq", !{!"result"}, i32 4}} {
    %vx = load i32, i32* @x
    %vy = load i32, i32* @y
    %a = add i32 %vx, %vy
    ret i32 %a
}
