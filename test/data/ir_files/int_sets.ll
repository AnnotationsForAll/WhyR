define void @int_set_1() !whyr.ensures !{!{!"eq", !{!"set", !{!"typeof", i32 0}}, !{!"set", !{!"typeof", i32 0}, i32 1, i32 2, i32 5}}} {
    ret void
}