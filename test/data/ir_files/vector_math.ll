define <2 x i32> @vadd() !whyr.ensures !{!{!"war", !"result == (vector){(i32)4,(i32)6}"}} {
    %a = add nuw nsw <2 x i32> <i32 1, i32 2>, <i32 3, i32 4>
    ret <2 x i32> %a
}

define <2 x i32> @vsub() !whyr.ensures !{!{!"war", !"result == (vector){(i32)2,(i32)2}"}} {
    %a = sub <2 x i32> <i32 3, i32 4>, <i32 1, i32 2>
    ret <2 x i32> %a
}

define <2 x i32> @vmul() !whyr.ensures !{!{!"war", !"result == (vector){(i32)3,(i32)8}"}} {
    %a = mul nsw <2 x i32> <i32 1, i32 2>, <i32 3, i32 4>
    ret <2 x i32> %a
}

define <2 x i32> @vdiv() !whyr.ensures !{!{!"war", !"result == (vector){(i32)0,(i32)0}"}} {
    %a = sdiv <2 x i32> <i32 1, i32 2>, <i32 3, i32 4>
    ret <2 x i32> %a
}

define <2 x i32> @vrem() !whyr.ensures !{!{!"war", !"result == (vector){(i32)1,(i32)2}"}} {
    %a = srem <2 x i32> <i32 1, i32 2>, <i32 3, i32 4>
    ret <2 x i32> %a
}
