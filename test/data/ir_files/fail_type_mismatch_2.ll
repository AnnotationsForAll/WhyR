define void @llvm_int_mismatch() !whyr.requires !{!{!"eq", i32 42, !{!"int", !"42"}}} {
    ret void
}
