define void @main() !whyr.requires !{!{!"forall", !{!{!"x", !{!"typeof", i32 0}}, !{!"y", !{!"typeof", i32 0}}}, !{!"eq", !{!"local", !"x"}, !{!"local", !"y"}}}} !whyr.ensures !{!{!"eq", i32 1, i32 2}} {
    ret void
}