define void @stor(i32* %p) !whyr.ensures !{!{!"eq", !{!"load", !{!"arg", !"p"}}, i32 4}} {
    store i32 4, i32* %p
    ret void
}

define void @stor_war(i32* %p) !whyr.ensures !{!{!"war", !"*%p == (i32)4"}} {
    store i32 4, i32* %p
    ret void
}
