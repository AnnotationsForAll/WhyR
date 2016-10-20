define i32 @llvm_result_mismatch() !whyr.requires !{!{!{!"eq", !{!"file_name", !"42", !"4", !"8"}, !{!"no_line_info"}}, !{!"result"}, !{!"int", !"42"}}} {
    ret i32 42
}
