define <4 x i32> @ivec() !whyr.ensures !{!{!"war", !"result == (vector){(i32)0,(i32)1,(i32)2,(i32)3}"}} {
    ret <4 x i32> <i32 0,i32 1,i32 2,i32 3>
}

define <4 x float> @fvec() !whyr.ensures !{!{!"war", !"result == (vector){(float)0.0,(float)1.0,(float)2.0,(float)3.0}"}} {
    ret <4 x float> <float 0.0,float 1.0,float 2.0,float 3.0>
}

define <4 x [ 0 x i32 ]*> @pvec() !whyr.ensures !{!{!"war", !"result == (vector){(i32[]*)null,(i32[]*)null,(i32[]*)null,(i32[]*)null}"}} {
    ret <4 x [ 0 x i32 ]*> zeroinitializer
}
