define void @non_bool_clause() !whyr.requires !{!{!"int", !"42"}} {
    ret void
}
