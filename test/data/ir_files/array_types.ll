define [ 4 x i8 ] @ret_4char() !whyr.ensures !{!{!"war", !"result[0] == result[0]"}} {
    ret [ 4 x i8 ] c"123\00"
}

define [ 3 x float ] @ret_3float() !whyr.ensures !{!{!"war", !"result[1] == (float)(-1.0)"}} {
    ret [ 3 x float ] [ float 0.0, float -1.0, float 3.5 ]
}

define [ 3 x float ] @ret_3float_exact() !whyr.ensures !{!{!"war", !"result == {(float)0.0,(float)-1.0,(float)3.5}"}} {
    ret [ 3 x float ] [ float 0.0, float -1.0, float 3.5 ]
}
