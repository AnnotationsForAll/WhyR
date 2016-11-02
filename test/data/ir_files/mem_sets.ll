define void @mem_set_1() !whyr.ensures !{!{!"in", !{!"set", !{!"typeof", i32* null}, i32* null}, i32* null}} {
    ret void
}

define void @mem_set_2() !whyr.ensures !{!{!"war", !"(set) {(i32*)null} == (set) {(i32*)null} offset (0..1)"}} {
    ret void
}
