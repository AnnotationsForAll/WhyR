@a = global i32 0

define void @func() !whyr.ensures !{!{!"war", !"old *@a != *@a"}} {
    %a = load i32, i32* @a
    %c = icmp eq i32 %a, 0
    %v = select i1 %c, i32 1, i32 0
    store i32 %v, i32* @a
    ret void
}