define i32 @ind_br() !whyr.ensures !{!{!"war", !"result == (i32)5"}} {
    %label = select i1 true, i8* blockaddress(@ind_br, %BlockTwo), i8* blockaddress(@ind_br, %BlockOne)
    indirectbr i8* %label, [ label %BlockOne, label %BlockTwo, label %BlockThree ]
    BlockOne:
    ret i32 0
    BlockTwo:
    ret i32 5
    BlockThree:
    ret i32 10
}
