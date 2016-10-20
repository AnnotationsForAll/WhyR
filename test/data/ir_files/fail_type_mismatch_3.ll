define i32 @llvm_result_mismatch() !whyr.requires !{!{!"eq", !{!"result"}, !{!"int", !"42"}}} {
    ret i32 42
}
