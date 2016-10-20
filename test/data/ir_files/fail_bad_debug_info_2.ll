define i32 @llvm_result_mismatch() !whyr.requires !{!{!{!{!"eq"}}, !{!"result"}, i32 42}} {
    ret i32 42
}
