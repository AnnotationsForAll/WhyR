define <4 x i1> @vicmp_eq() !whyr.ensures !{!{!"war", !"result == (vector){(i1)1,(i1)1,(i1)1,(i1)1}"}} {
    %a = icmp eq <4 x i32> zeroinitializer, zeroinitializer
    ret <4 x i1> %a
}

define <4 x i1> @vicmp_gt() !whyr.ensures !{!{!"war", !"result == (vector){(i1)0,(i1)0,(i1)0,(i1)0}"}} {
    %a = icmp ugt <4 x i32> zeroinitializer, zeroinitializer
    ret <4 x i1> %a
}

define <4 x i1> @vfcmp() !whyr.ensures !{!{!"war", !"result == (vector){(i1)1,(i1)1,(i1)1,(i1)1}"}} {
    %a = fcmp uno <4 x double> zeroinitializer, zeroinitializer
    ret <4 x i1> %a
}

define <4 x i1> @vfcmp_true() !whyr.ensures !{!{!"war", !"result == (vector){(i1)1,(i1)1,(i1)1,(i1)1}"}} {
    %a = fcmp true <4 x double> zeroinitializer, zeroinitializer
    ret <4 x i1> %a
}