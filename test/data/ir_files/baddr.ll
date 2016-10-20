define i8* @own_label() !whyr.ensures !{!{!"eq", !{!"result"}, !{!"blockaddress", !"own_label", !"OwnLabel"}}} {
    ret i8* blockaddress(@own_label, %OwnLabel)
    OwnLabel:
    unreachable
}
