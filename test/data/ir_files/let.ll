define void @main() !whyr.ensures !{!{!"let", !{!{!"x", !{!"typeof", i32 0}, i32 2}, !{!"y", !{!"typeof", i32 0}, i32 2}}, !{!"eq", !{!"local", !"x"}, !{!"local", !"y"}}}} {
    ret void
}