define void @too_big_clause() !whyr.requires !{!"and", !{!"true"}, !{!"false"}} {
    ret void
}
