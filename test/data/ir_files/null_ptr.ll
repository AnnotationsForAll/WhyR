define i8* @ret_null() !whyr.ensures !{!{!"eq", !{!"result"}, i8* null}} {
    ret i8* null
}

define i8* @ret_null_war() !whyr.ensures !{!{!"war", !"result == (i8*) null"}} {
    ret i8* null
}
