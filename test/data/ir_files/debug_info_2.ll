define i32 @llvm_result_mismatch() !whyr.requires !{!{!{!"eq", !{!"file_name", !"42", !"4", !"8"}}, !{!"result"}, i32 42}} {
    ret i32 42
}
