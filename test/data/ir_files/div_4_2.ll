define i2 @u() !whyr.ensures !{!{!"eq", !{!"result"}, i2 1}} {
    %a = udiv i2 3, 2
    ret i2 %a
}

define i2 @s() !whyr.ensures !{!{!"eq", !{!"result"}, i2 0}} {
    %a = sdiv i2 3, 2
    ret i2 %a
}