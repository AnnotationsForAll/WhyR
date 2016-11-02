@a = global i32 0
@b = global i32 0

define void @set_a() !whyr.assigns !{!{!"war", !"(set) {@a}"}} {
    store i32 1, i32* @a
    ret void
}

define void @set_a_b() !whyr.ensures !{!{!"war", !"*@b == (i32)4"}} {
    store i32 4, i32* @b
    call void() @set_a()
    ret void
}
