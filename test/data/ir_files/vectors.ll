define <4 x i32> @ivec() {
    ret <4 x i32> <i32 0,i32 1,i32 2,i32 3>
}

define <4 x float> @fvec() {
    ret <4 x float> <float 0.0,float 1.0,float 2.0,float 3.0>
}

define <4 x [ 0 x i32 ]*> @pvec() {
    ret <4 x [ 0 x i32 ]*> zeroinitializer
}
