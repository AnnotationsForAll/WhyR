define <4 x i1> @vicmp_eq() {
    %a = icmp eq <4 x i32> zeroinitializer, zeroinitializer
    ret <4 x i1> %a
}

define <4 x i1> @vicmp_gt() {
    %a = icmp ugt <4 x i32> zeroinitializer, zeroinitializer
    ret <4 x i1> %a
}

define <4 x i1> @vfcmp() {
    %a = fcmp uno <4 x double> zeroinitializer, zeroinitializer
    ret <4 x i1> %a
}

define <4 x i1> @vfcmp_true() {
    %a = fcmp true <4 x double> zeroinitializer, zeroinitializer
    ret <4 x i1> %a
}