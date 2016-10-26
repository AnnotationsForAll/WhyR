define <4 x i32> @vtrunc() {
    %a = trunc <4 x i64> <i64 8, i64 2, i64 77, i64 0> to <4 x i32>
    ret <4 x i32> %a
}

define <4 x i32> @vfptoui() {
    %a = fptoui <4 x float> <float -1.0, float 0.0, float 1.0, float 22.0> to <4 x i32>
    ret <4 x i32> %a
}

define <4 x float> @vuitofp() {
    %a = uitofp <4 x i32> <i32 0, i32 2, i32 1, i32 4> to <4 x float>
    ret <4 x float> %a
}
