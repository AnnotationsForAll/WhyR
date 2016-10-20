declare i32 @f()
declare i32 @g()

define i32 @main() {
    %a = call i32 @f()
    %b = call i32 @g()
    %c = add i32 %a, %b
    ret i32 %c
}
