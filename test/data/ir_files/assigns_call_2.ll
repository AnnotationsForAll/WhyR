@a = global i32 0
@b = global i32 0

define void @set_a() !whyr.assigns !{!{!"war", !"(set) {@a}"}} {
    store i32 1, i32* @a
    ret void
}

define void @set_a_b() !whyr.assigns !{!{!"war", !"(set) {@a,@b}"}} {
    store i32 4, i32* @b
    call void() @set_a()
    ret void
}
