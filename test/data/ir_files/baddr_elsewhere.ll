define i8* @get_baddr() !whyr.ensures !{!{!"eq", !{!"result"}, i8* blockaddress(@rando, %RandoLabel)}} {
    ret i8* blockaddress(@rando, %RandoLabel)
}

define void @rando() {
    unreachable
RandoLabel:
    unreachable
}
